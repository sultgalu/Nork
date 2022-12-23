#pragma once

#include "Modules/Renderer/Vulkan/CommandBuffer.h"
namespace Nork::Renderer::Vulkan {
	class TimelineSemaphore: public vk::raii::Semaphore
	{
	public:
		TimelineSemaphore(uint64_t initial = 0)
			: TimelineSemaphore(vk::SemaphoreTypeCreateInfo(vk::SemaphoreType::eTimeline, initial))
		{}
	private:
		TimelineSemaphore(const vk::SemaphoreTypeCreateInfo& typeInfo)
			: vk::raii::Semaphore(Device::Instance(), vk::SemaphoreCreateInfo({}, & typeInfo)),
			initialValue(typeInfo.initialValue)
		{}
	public:
		bool Wait(uint64_t val, std::chrono::nanoseconds timeout = std::chrono::seconds(1))
		{
			vk::Result res = Device::Instance().waitSemaphores(vk::SemaphoreWaitInfo({}, **this, val), timeout.count());
			if (res == vk::Result::eSuccess)
				return true;
			Logger::Error("waitSemaphores returned ", vk::to_string(res));
			return false;
		}
		bool IsSignaled(uint64_t val)
		{
			return Device::Instance().waitSemaphores(vk::SemaphoreWaitInfo({}, **this, val), 0)
				== vk::Result::eSuccess;
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
	public:
		const uint64_t initialValue;
	};
}