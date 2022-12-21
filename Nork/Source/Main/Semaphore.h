#pragma once

#include "Modules/Renderer/Vulkan/CommandBuffer.h"
namespace Nork::Renderer::Vulkan {
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
		vkCreateSemaphore(*Device::Instance(), &createInfo, nullptr, &handle);
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

			vkCreateSemaphore(*Device::Instance(), &createInfo, nullptr, &handle);
		}
		struct SubmitInfo
		{
			SubmitInfo(uint64_t wait, uint64_t signal)
				: wait(wait), signal(signal)
			{
				VkTimelineSemaphoreSubmitInfo timelineInfo1{};
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
			vkDestroySemaphore(*Device::Instance(), handle, nullptr);
		}
	public:
		VkSemaphore handle;
	};
}