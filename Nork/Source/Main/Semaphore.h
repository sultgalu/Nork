#pragma once

#include "CommandBuffer.h"

static VkSemaphore CreateTimelineSemaphore(uint64_t initial = 0)
{
	VkSemaphoreTypeCreateInfo timelineCreateInfo{};
	timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	timelineCreateInfo.initialValue = initial;

	VkSemaphoreCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = &timelineCreateInfo;
	createInfo.flags = 0;

	VkSemaphore handle;
	vkCreateSemaphore(Device::Instance().device, &createInfo, nullptr, &handle);
	return handle;
}
class TimelineSemaphore
{
public:
	TimelineSemaphore(uint64_t initial = 0)
	{
		VkSemaphoreTypeCreateInfo timelineCreateInfo{};
		timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = initial;

		VkSemaphoreCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		createInfo.pNext = &timelineCreateInfo;
		createInfo.flags = 0;

		vkCreateSemaphore(Device::Instance().device, &createInfo, nullptr, &handle);
	}
	struct SubmitInfo
	{
		SubmitInfo(uint64_t wait, uint64_t signal)
			: wait(wait), signal(signal) 
		{
			VkTimelineSemaphoreSubmitInfo timelineInfo1 {};
			timelineInfo1.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
			timelineInfo1.waitSemaphoreValueCount = 1;
			timelineInfo1.pWaitSemaphoreValues = &wait;
			timelineInfo1.signalSemaphoreValueCount = 1;
			timelineInfo1.pSignalSemaphoreValues = &signal;
		}
		VkTimelineSemaphoreSubmitInfo info;
		uint64_t wait, signal;
	};
	static VkTimelineSemaphoreSubmitInfo SubmitInfo(uint64_t wait, uint64_t signal)
	{
		return SubmitInfo(wait, signal);
	}
	~TimelineSemaphore()
	{
		vkDestroySemaphore(Device::Instance().device, handle, nullptr);
	}
public:
	VkSemaphore handle;
};

class Batch
{
public:
	Batch(CommandPool& pool)
		: cmdbuf(pool.CreateCommandBuffer()), signalSem(CreateTimelineSemaphore())
	{
	}
	virtual void Setup() = 0;
public:
	CommandBuffer cmdbuf;
	VkSemaphore signalSem;
	std::vector<std::shared_ptr<Batch>> dependencies;
};

class Frame
{
	VkQueue queue;
	std::vector<VkSubmitInfo> submitInfos;
public:
	Frame(VkQueue queue, const std::vector<std::shared_ptr<Batch>>& order)
		: order(order), queue(queue)
	{
		batches.reserve(order.size());
		submitInfos.reserve(order.size());
		for (auto& o : order)
		{
			auto& batch = batches.emplace_back(*o);
			submitInfos.push_back(batch.SubmitInfo());
		}
	}
	void SetupCommandBuffers()
	{
		for (auto& batch : order)
			batch->Setup();
	}
	void Submit(VkFence fence = VK_NULL_HANDLE)
	{
		vkQueueSubmit(queue, submitInfos.size(), submitInfos.data(), fence) == VkSuccess();
	}
private:
	struct Batch2
	{
		Batch2(Batch& batch)
			: waitSemValues(batch.dependencies.size(), SIGNALED)
		{
			waitSemHandles.reserve(batch.dependencies.size());
			for (auto& dep : batch.dependencies)
				waitSemHandles.push_back(dep->signalSem);

			semInfo = VkTimelineSemaphoreSubmitInfo{};
			semInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
			semInfo.waitSemaphoreValueCount = waitSemValues.size();
			semInfo.pWaitSemaphoreValues = waitSemValues.data();
			semInfo.signalSemaphoreValueCount = 1;
			semInfo.pSignalSemaphoreValues = &SIGNALED;

			submitInfo = VkSubmitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pNext = &semInfo;
			submitInfo.waitSemaphoreCount = waitSemHandles.size();
			submitInfo.pWaitSemaphores = waitSemHandles.data();
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &batch.signalSem;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &batch.cmdbuf.handle;
		}
		const VkSubmitInfo& SubmitInfo()
		{
			return submitInfo;
		}
		std::vector<VkSemaphore> waitSemHandles;
		std::vector<uint64_t> waitSemValues;
		VkTimelineSemaphoreSubmitInfo semInfo;
		VkSubmitInfo submitInfo;
		// static constexpr uint64_t UNSIGNALED = 0;
		static constexpr uint64_t SIGNALED = 1;
	};
public:
	std::vector<std::shared_ptr<Batch>> order;
	std::vector<Batch2> batches;
};