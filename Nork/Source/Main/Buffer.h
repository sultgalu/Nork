#pragma once

#include "VkSuccess.h"
#include "Device.h"
#include "DeviceMemory.h"

class Buffer
{
public:
    Buffer(const Buffer&) = delete;
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memFlags, bool autoMap = false)
	{
        bufferInfo = VkBufferCreateInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateBuffer(Device::Instance().device, &bufferInfo, nullptr, &handle) == VkSuccess();

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(Device::Instance().device, handle, &memRequirements);
        memory = std::make_shared<DeviceMemory>(memRequirements.size, memRequirements.memoryTypeBits, memFlags);

        memOffset = 0;
        vkBindBufferMemory(Device::Instance().device, handle, memory->handle, memOffset);
        if (autoMap)
            memory->Map();
	}
    ~Buffer()
    {
        vkDestroyBuffer(Device::Instance().device, handle, nullptr);
    }
    void* Ptr()
    {
        if (!memory->IsMapped())
            std::unreachable();
        return memory->ptr;
    }
    template<class T>
    void operator=(const T& val)
    {
        *((T*)Ptr()) = val;
    }
public:
    VkBuffer handle;
    VkBufferCreateInfo bufferInfo;

    std::shared_ptr<DeviceMemory> memory;
    VkDeviceSize memOffset;
};