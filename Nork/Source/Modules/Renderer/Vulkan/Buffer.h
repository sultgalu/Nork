#pragma once

#include "Device.h"
#include "DeviceMemory.h"

namespace Nork::Renderer::Vulkan {
    class Buffer
    {
    public:
        Buffer(const Buffer&) = delete;
        Buffer(VkDeviceSize size, VkBufferUsageFlags usage, vk::MemoryPropertyFlags memFlags, bool autoMap = false)
        {
            bufferInfo = VkBufferCreateInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            vkCreateBuffer(*Device::Instance(), &bufferInfo, nullptr, &handle) == VkSuccess();

            VkMemoryRequirements memreq;
            vkGetBufferMemoryRequirements(*Device::Instance(), handle, &memreq);
            memory = std::make_shared<DeviceMemory>(vk::MemoryAllocateInfo(memreq.size,
                Device::Instance().findMemoryType(memreq.memoryTypeBits, memFlags)));

            memOffset = 0;
            vkBindBufferMemory(*Device::Instance(), handle, **memory, memOffset);
            if (autoMap)
                memory->Map();
        }
        ~Buffer()
        {
            vkDestroyBuffer(*Device::Instance(), handle, nullptr);
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
}