#pragma once
#include "Device.h"
#include "VkSuccess.h"

class DeviceMemory
{
public:
	DeviceMemory(VkDeviceSize size, uint32_t memTypeBits, VkMemoryPropertyFlags properties)
	{
		allocInfo = VkMemoryAllocateInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = size;
		allocInfo.memoryTypeIndex = Device::Instance().findMemoryType(memTypeBits, properties);

		vkAllocateMemory(Device::Instance().device, &allocInfo, nullptr, &handle) == VkSuccess();
	}
	~DeviceMemory()
	{
		vkFreeMemory(Device::Instance().device, handle, nullptr);
	}
	void* Map()
	{
		return Map(0, allocInfo.allocationSize);
	}
	void* Map(VkDeviceSize offset, VkDeviceSize size)
	{
		if (IsMapped())
			std::unreachable();
		vkMapMemory(Device::Instance().device, handle, offset, size, 0, &ptr) == VkSuccess();
		return ptr;
	}
	void Unmap()
	{
		vkUnmapMemory(Device::Instance().device, handle);
		ptr = nullptr;
	}
	bool IsMapped()
	{
		return ptr != nullptr;
	}
public:
	void* ptr = nullptr;
	VkDeviceMemory handle;
	VkMemoryAllocateInfo allocInfo;
};