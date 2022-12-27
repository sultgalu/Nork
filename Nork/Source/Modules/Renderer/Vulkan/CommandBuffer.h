#pragma once

#include "Pipeline.h"
#include "DescriptorSet.h"
#include "Framebuffer.h"

namespace Nork::Renderer::Vulkan {
    class CommandPool: public vk::raii::CommandPool
    {
    public:
        CommandPool(vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            :vk::raii::CommandPool(Device::Instance(), vk::CommandPoolCreateInfo(flags,
                PhysicalDevice::Instance().graphicsQueueFamily))
        {}
    private:
        friend class CommandBuffer;
        std::vector<vk::CommandBuffer> CreateCommandBuffers(uint32_t count)
        {
            vk::raii::CommandBuffers buffers(Device::Instance(),
                vk::CommandBufferAllocateInfo(**this, vk::CommandBufferLevel::ePrimary, count));
            std::vector<vk::CommandBuffer> handles;
            for (auto& raii : buffers)
                handles.push_back(raii.release());
            return handles;
        }
        vk::CommandBuffer CreateCommandBuffer()
        {
            return CreateCommandBuffers(1)[0];
        }
    };

    class CommandBuffer : public vk::raii::CommandBuffer
    {
    public:
        CommandBuffer(std::shared_ptr<CommandPool> pool, vk::CommandBuffer handle)
            : pool(pool), vk::raii::CommandBuffer(Device::Instance(), handle, **pool)
        {}
    public:
        CommandBuffer(CommandBuffer&& other)
            :vk::raii::CommandBuffer(std::forward<vk::raii::CommandBuffer>(other))
        {}
        ~CommandBuffer()
        {
            clear(); // make sure cmd gets destroyed before pool
        }
        CommandBuffer(const std::shared_ptr<CommandPool>& pool)
            : CommandBuffer(pool, pool->CreateCommandBuffer())
        {}
        static std::vector<CommandBuffer> Create(std::shared_ptr<CommandPool> pool, uint32_t count)
        {
            auto handles = pool->CreateCommandBuffers(count);
            std::vector<CommandBuffer> cmds;
            cmds.reserve(count);
            for(auto& handle: handles)
                cmds.emplace_back(pool, handle);
            return std::move(cmds);
        }
        void DrawQuad()
        {
            this->draw(6, 1, 0, 0);
        }
        std::shared_ptr<CommandPool> pool;
    };

}