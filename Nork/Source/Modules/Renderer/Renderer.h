#pragma once

#include "Resources.h"
#include "EditorPass.h"
#include "DeferredPass.h"

namespace Nork::Renderer {

struct Client
{
	// return the number of elements written to 'commands'
	virtual uint32_t FillDrawBuffers(std::span<DrawParams> params,
		std::span<vk::DrawIndexedIndirectCommand> commands) = 0;
	virtual void FillLightBuffers(std::span<uint32_t> dIdxs, std::span<uint32_t> pIdxs,
		uint32_t& dlCount, uint32_t& dsCount, uint32_t& plCount, uint32_t& psCount) = 0;
	// do stuff that requires a commandBuffer
	virtual void OnCommands() = 0;
};

class Renderer
{
public:
	Renderer()
		: allocator(Vulkan::PhysicalDevice::Instance())
	{
		instance = this;
		using namespace Vulkan;
		Commands::Instance().Begin(0); // begin initialize phase

		resources = std::make_unique<Resources>();
		createSyncObjects();
		renderPasses.push_back(std::make_shared<DeferredPass>());
		renderPasses.push_back(std::make_shared<EditorPass>());

		Commands::Instance().End();
		Commands::Instance().Submit(); // end initialize phase
	}
	~Renderer()
	{
		using namespace Vulkan;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(*Device::Instance(), imageAvailableSemaphores[i], nullptr);
		}
	}
	uint32_t invalid_frame_idx = UINT32_MAX;
	uint32_t BeginFrame()
	{
		using namespace Vulkan;

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		Commands::Instance().Begin(currentFrame);

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
			//swapChain->recreateSwapChain();
			return invalid_frame_idx;
		}
		else if (result.first != vk::Result::eSuccess && result.first != vk::Result::eSuboptimalKHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		return result.second;
	}
	void EndFrame(uint32_t imgIdx)
	{
		using namespace Vulkan;

		Commands::Instance().Submit(true, { imageAvailableSemaphores[currentFrame] });

		auto res = Device::Instance().graphicsQueue.presentKHR(
			vk::PresentInfoKHR()
			.setWaitSemaphores(**Commands::Instance().renderFinishedSemaphores2[currentFrame])
			.setSwapchains(*SwapChain::Instance())
			.setImageIndices(imgIdx)
		);
		if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || framebufferResized)
		{
			framebufferResized = false;
			//swapChain->recreateSwapChain();
		}
		else if (res != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
	}
	void DrawFrame()
	{
		auto imgIdx = BeginFrame();
		if (imgIdx == invalid_frame_idx)
			return;
		for (auto& pass : renderPasses)
		{
			pass->recordCommandBuffer(*Commands::Instance().cmds[currentFrame], imgIdx, currentFrame);
		}
		Commands::Instance().End();
		EndFrame(imgIdx);
	}
	void createSyncObjects()
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
public:
	MemoryAllocator allocator;
	Commands commands = Commands(MAX_FRAMES_IN_FLIGHT);
	MemoryTransfer transfer;
	std::unique_ptr<Resources> resources;
	std::vector<std::shared_ptr<RenderPass>> renderPasses;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	uint32_t currentFrame = 0;
	bool framebufferResized = false;

	Client* client = nullptr;

	inline static uint32_t MAX_FRAMES_IN_FLIGHT = 2;

public:
	static Renderer& Instance()
	{
		return *instance;
	}
	static Renderer* instance;
};
}