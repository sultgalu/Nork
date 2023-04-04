#pragma once
#include "Modules/Renderer/Vulkan/Device.h"

namespace Nork::Renderer::Vulkan {
	class DeviceMemory : public vk::raii::DeviceMemory
	{
	public:
		DeviceMemory(const vk::MemoryAllocateInfo& allocInfo)
			:allocInfo(allocInfo), vk::raii::DeviceMemory(Device::Instance(), allocInfo)
		{}
		void* Map()
		{
			if (IsMapped())
				std::unreachable();
			ptr = mapMemory(0, allocInfo.allocationSize);
			return ptr;
		}
		bool IsMapped() const
		{
			return ptr != nullptr;
		}
	public:
		void* ptr = nullptr;
		VkMemoryAllocateInfo allocInfo;
	};
}