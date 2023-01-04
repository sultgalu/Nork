#include "Renderer.h"
#include "Vulkan/Window.h"

namespace Nork::Renderer {

static std::weak_ptr<EditorPass> editorPass;
Renderer::Renderer()
	: allocator(Vulkan::PhysicalDevice::Instance())
{
	instance = this;
	using namespace Vulkan;
	Commands::Instance().BeginTransferCommandBuffer(); // begin initialize phase

	resources = std::make_unique<Resources>();
	createSyncObjects();
	renderPasses.push_back(std::make_shared<ShadowMapPass>());
	renderPasses.push_back(std::make_shared<DeferredPass>());
	renderPasses.push_back(std::make_shared<EditorPass>());

	editorPass = std::reinterpret_pointer_cast<EditorPass>(renderPasses.back());
	Vulkan::Window::Instance().onFbResize = [&](int w, int h)
	{
		framebufferResized = true;
	};

	Commands::Instance().EndTransferCommandBuffer();
	Commands::Instance().SubmitTransferCommands(); // end initialize phase
}
Renderer::~Renderer()
{
	using namespace Vulkan;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(*Device::Instance(), imageAvailableSemaphores[i], nullptr);
	}
}
static uint32_t invalid_frame_idx = UINT32_MAX;
static void GenerateCubeViewProjections(glm::mat4* vps, float near, float far, const glm::vec3& pos)
{
	glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.0f, near, far);
	
	int k = 0;
	vps[k++] = glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	vps[k++] = glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	vps[k++] = glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	vps[k++] = glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	vps[k++] = glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	vps[k++] = glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	for (int i = 0; i < 6; i++)
	{
		vps[i] = projection * vps[i];
	}
}
uint32_t Renderer::BeginFrame()
{
	if (framebufferResized)
	{
		auto ep = editorPass.lock();
		if (ep)
		{
			auto inFlightFb = ep->fbUI;
			Commands::Instance().OnRenderFinished([inFlightFb]() {});
			ep->CreateFramebuffer();
		}
		framebufferResized = false;
	}
	using namespace Vulkan;

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	Commands::Instance().NextFrame(currentFrame);
	Commands::Instance().BeginRenderCommandBuffer();
	Commands::Instance().BeginTransferCommandBuffer();
	// start CPU expensive stuff that does not yet require the next image
	resources->OnNewFrame(currentFrame);
	if (client)
	{
		auto getPtr = [&](HostWritableBuffer& buf)
		{
			return ((uint8_t*)buf.Ptr() + resources->DynamicOffset(buf));
		};
		client->OnCommands();
		auto dPtr = (uint32_t*)getPtr(*resources->dirLightParams);
		auto pPtr = (uint32_t*)getPtr(*resources->pointLightParams);
		auto dIdxs = std::span(&dPtr[4], resources->DynamicSize(*resources->dirLightParams) / sizeof(uint32_t) - 4);
		auto pIdxs = std::span(&pPtr[4], resources->DynamicSize(*resources->pointLightParams) / sizeof(uint32_t) - 4);
		client->FillLightBuffers(dIdxs, pIdxs, dPtr[0], dPtr[1], pPtr[0], pPtr[1]);

		resources->shadowMaps.clear(); resources->shadowMapsCube.clear();
		client->ProvideShadowMapsForUpdate(resources->shadowMaps, resources->shadowMapsCube);
		auto pShadVps = (glm::mat4*)getPtr(*resources->pShadowVps);
		for (auto& shadowMap : resources->shadowMapsCube)
		{
			auto& shadow = shadowMap->Shadow();
			GenerateCubeViewProjections(pShadVps, shadow->near, shadow->far, shadowMap->position);
			pShadVps += 6;
		}

		auto paramsPtr = (DrawParams*)getPtr(*resources->drawParams);
		auto params = std::span(paramsPtr, resources->DynamicSize(*resources->drawParams) / sizeof(DrawParams));
		auto commandsOffs = resources->DynamicOffset(*resources->drawCommands);
		auto commandsPtr = (vk::DrawIndexedIndirectCommand*)getPtr(*resources->drawCommands);
		auto commands = std::span(commandsPtr, resources->DynamicSize(*resources->drawCommands) / sizeof(vk::DrawIndexedIndirectCommand));
		resources->drawCommandCount = client->FillDrawBuffers(params, commands);
	}
	resources->FlushWrites();
	// end
	auto result = SwapChain::Instance().acquireNextImage(UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE);

	if (result.first == vk::Result::eErrorOutOfDateKHR)
	{
		return invalid_frame_idx;
	}
	else if (result.first != vk::Result::eSuccess && result.first != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	return result.second;
}
void Renderer::EndFrame(uint32_t imgIdx)
{
	using namespace Vulkan;

	Commands::Instance().SubmitTransferCommands();
	Commands::Instance().SubmitRenderCommands(true, { imageAvailableSemaphores[currentFrame] });

	auto res = Device::Instance().graphicsQueue.presentKHR(
		vk::PresentInfoKHR()
		.setWaitSemaphores(**Commands::Instance().renderFinishedSemaphores2[currentFrame])
		.setSwapchains(*SwapChain::Instance())
		.setImageIndices(imgIdx)
	);
	if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR)
	{
		std::unreachable();
	}
	else if (res != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}
}
void Renderer::DrawFrame()
{
	auto imgIdx = BeginFrame();
	if (imgIdx == invalid_frame_idx)
		return;
	for (auto& pass : renderPasses)
	{
		pass->RecordCommandBuffer(*Commands::Instance().renderCmds[currentFrame], imgIdx, currentFrame);
	}
	Commands::Instance().EndRenderCommandBuffer();
	Commands::Instance().EndTransferCommandBuffer();
	EndFrame(imgIdx);
}
void Renderer::createSyncObjects()
{
	using namespace Vulkan;
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // create as already signaled (first wait wont deadlock)

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkCreateSemaphore(*Device::Instance(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VkSuccess();
	}
}
}