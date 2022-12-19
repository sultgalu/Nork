#pragma once
#include "Device.h"
#include "VkSuccess.h"

class DeviceMemory: public vk::raii::DeviceMemory
{
public:
	DeviceMemory(const vk::MemoryAllocateInfo& allocInfo)
		:allocInfo(allocInfo), vk::raii::DeviceMemory(Device::Instance2(), allocInfo)
	{}
	void* Map()
	{
		if (IsMapped())
			std::unreachable();
		ptr = mapMemory(0, allocInfo.allocationSize);
		return ptr;
	}
	bool IsMapped()
	{
		return ptr != nullptr;
	}
public:
	void* ptr = nullptr;
	VkMemoryAllocateInfo allocInfo;
};