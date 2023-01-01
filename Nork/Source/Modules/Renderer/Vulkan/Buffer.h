#pragma once

#include "Device.h"
#include "DeviceMemory.h"

namespace Nork::Renderer::Vulkan {
    struct BufferCreateInfo: vk::BufferCreateInfo
    {
        BufferCreateInfo(vk::DeviceSize size, vk::BufferUsageFlags usage)
            : vk::BufferCreateInfo({}, size, usage) {}
    };
    class Buffer: public vk::raii::Buffer
    {
    public:
        Buffer(const Buffer&) = delete;
        Buffer(const BufferCreateInfo& createInfo)
            :vk::raii::Buffer(Device::Instance(), createInfo), createInfo(createInfo)
        {}
        void BindMemory(std::shared_ptr<DeviceMemory> memory, vk::DeviceSize offset)
        {
            bindMemory(**memory, offset);
            this->memory = memory;
        }
    private:
        using vk::raii::Buffer::bindMemory;
    public:
        BufferCreateInfo createInfo;
        std::shared_ptr<DeviceMemory> memory = nullptr;
    };
}