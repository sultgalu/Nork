#include "Renderer.h"
#include "Vulkan/Window.h"
#include "DeferredPass.h"
#include "PostProcessPass.h"
#include "ShadowMapPass.h"

namespace Nork::Renderer {

Renderer::Renderer()
	: allocator(Vulkan::PhysicalDevice::Instance())
{
	instance = this;
	using namespace Vulkan;
	Commands::Instance().BeginTransferCommandBuffer(); // begin initialize phase

	resources = std::make_unique<Resources>();
	CreateSyncObjects();
	renderPasses.push_back(std::make_shared<ShadowMapPass>());
	auto deferredPass = std::make_shared<DeferredPass>();
	mainImage = deferredPass->fbColor;
	renderPasses.push_back(deferredPass);
	renderPasses.push_back(std::make_shared<PostProcessPass>(mainImage));

	Vulkan::Window::Instance().onFbResize = [&](int w, int h) {
		framebufferResized = true;
	};

	Commands::Instance().EndTransferCommandBuffer();
	Commands::Instance().SubmitTransferCommands(); // end initialize phase

	deviceDatas.push_back(resources->indexBuffer);
	deviceDatas.push_back(resources->vertexBuffer);
	deviceDatas.push_back(resources->modelMatrices);
	deviceDatas.push_back(resources->materials);
	deviceDatas.push_back(resources->dirLights);
	deviceDatas.push_back(resources->dirShadows);
	deviceDatas.push_back(resources->pointLights);
	deviceDatas.push_back(resources->pointShadows);
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
		for (auto& pass : renderPasses) {
			pass->OnFramebufferResized();
		}
		framebufferResized = false;
	}
	using namespace Vulkan;

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	Commands::Instance().ProcessFinishedCommandBufferCallbacks();
	Commands::Instance().NextFrame(currentFrame);
	Commands::Instance().BeginTransferCommandBuffer();
	Commands::Instance().BeginRenderCommandBuffer(); // force wait for rendering to finish before writing to buffers in-use
	 
	// begin CPU expensive stuff that does not yet require the next image
	for (auto& deviceData : deviceDatas) {
		deviceData->OnNewFrame();
	}
	resources->OnNewFrame(currentFrame);
	if (client)
	{
		auto getPtr = [&](HostVisibleBuffer& buf)
		{
			return ((uint8_t*)buf.memory.Ptr() + resources->DynamicOffset(buf));
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
		SetupDrawBuffers(client->GetObjectsToDraw(), params, commands);
	}
	for (auto& pass : renderPasses) {
		pass->OnTransferCommands();
	}
	for (auto& deviceData : deviceDatas) {
		deviceData->FlushWrites();
	}
	Commands::Instance().EndTransferCommandBuffer();

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

	Commands::Instance().SubmitRenderAndTransferCommands(true, { imageAvailableSemaphores[currentFrame] });

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
		pass->RecordCommandBuffer(*Commands::Instance().GetCurrentRenderCmd(), imgIdx, currentFrame);
	}
	Commands::Instance().EndRenderCommandBuffer();
	EndFrame(imgIdx);
}
void Renderer::SetupDrawBuffers(std::vector<Object>&& objects, std::span<DrawParams> params,
	std::span<vk::DrawIndexedIndirectCommand> commands)
{
	DrawCounts drawCounts;

	static std::vector<ToBeDrawn> tbdrawn;

	auto add = [&](const Object& obj, std::vector<ToBeDrawn>& to) {
		for (size_t i = 0; i < obj.model->nodes.size(); i++) {
			auto& node = obj.model->nodes[i];
			for (auto& prim : node.mesh->primitives) {
				to.push_back(ToBeDrawn{ .mesh = prim.meshData, .material = prim.material, .modelMatrix = obj.childTransforms[i], .shadingMode = prim.shadingMode });
			}
		}
	};
	for (auto obj : objects) {
		if (true) // eg.: its material has a deferredShader
		{
			add(obj, tbdrawn);
		}
		if (true) // eg.: not opaque
		{
			// add(dr, shadowedObjects);
		}
	}

	std::sort(tbdrawn.begin(), tbdrawn.end(), [](const ToBeDrawn& left, const ToBeDrawn& right) {
		if (left.shadingMode == right.shadingMode) {
			return left.mesh->vertices->offset < right.mesh->vertices->offset;
		}
		return left.shadingMode < right.shadingMode;
	});

	if (params.size() < tbdrawn.size()) {
		std::unreachable();
	}

	uint32_t commandCount = 0;
	uint32_t instanceCount = 0;
	vk::DrawIndexedIndirectCommand* lastCommand = nullptr;
	for (auto& tbd : tbdrawn) {
		params[instanceCount].mmIdx = tbd.modelMatrix->offset;
		params[instanceCount].matIdx = tbd.material->deviceData->offset;

		auto& mesh = tbd.mesh;

		if (commandCount > 0 && lastCommand->vertexOffset == mesh->vertices->offset
			&& lastCommand->firstIndex == mesh->indices->offset) // test if it is the same mesh reference (could expand it to vertex/index counts)
		{
			lastCommand->instanceCount++;
		}
		else {
			commands[commandCount].vertexOffset = mesh->vertices->offset;
			commands[commandCount].firstIndex = mesh->indices->offset;
			commands[commandCount].firstInstance = instanceCount;
			commands[commandCount].indexCount = mesh->indices->count;
			commands[commandCount].instanceCount = 1;
			lastCommand = &commands[commandCount];
			commandCount++;
			if (tbd.shadingMode == ShadingMode::Default) {
				drawCounts.defaults++;
			}
			else if (tbd.shadingMode == ShadingMode::Blend) {
				drawCounts.blend++;
			}
			else if (tbd.shadingMode == ShadingMode::Unlit) {
				drawCounts.unlit++;
			}
		}
		instanceCount++;
	}
	resources->drawCommandCount = drawCounts;
	tbdrawn.clear(); // let go of resource references
}
void Renderer::RefreshShaders()
{
	for (auto& pass : renderPasses) {
		pass->RefreshShaders();
	}
}
void Renderer::CreateSyncObjects()
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