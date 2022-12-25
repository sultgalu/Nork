#pragma once

#include "Device.h"
#include "DeviceMemory.h"
#include "../MemoryAllocator.h"

namespace Nork::Renderer::Vulkan {
    struct BufferCreateInfo: vk::BufferCreateInfo
    {
        BufferCreateInfo(vk::DeviceSize size, vk::BufferUsageFlagBits usage)
            : vk::BufferCreateInfo({}, size, usage) {}
    };
    class Buffer: public vk::raii::Buffer
    {
    public:
        Buffer(const Buffer&) = delete;
        Buffer(const BufferCreateInfo& createInfo, vk::MemoryPropertyFlags memFlags, bool autoMap = false)
            :vk::raii::Buffer(Device::Instance(), createInfo), createInfo(createInfo)
        {
            auto memreq = getMemoryRequirements();
            // memory = std::make_shared<DeviceMemory>(vk::MemoryAllocateInfo(memreq.size,
            //     Device::Instance().findMemoryType(memreq.memoryTypeBits, memFlags)));
            memory = MemoryAllocationWrapper(
                MemoryAllocator::Instance().Allocate(memreq, memFlags));

            bindMemory(*memory.DeviceMemory(), memory.Offset());
            if (autoMap)
                memory.Map();
        }
        void* Ptr()
        {
            return memory.Ptr();
        }
        template<class T>
        void operator=(const T& val)
        {
            *((T*)Ptr()) = val;
        }
    public:
        BufferCreateInfo createInfo;

        MemoryAllocationWrapper memory;
    };
}