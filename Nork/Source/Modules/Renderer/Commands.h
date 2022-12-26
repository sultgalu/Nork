#pragma once

#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Semaphore.h"

namespace Nork::Renderer {

    class Commands
    {
    public:
        struct Submit
        {
            Submit(const Submit&) = delete;
            Submit(const std::shared_ptr<Vulkan::CommandPool>& pool)
                : cmd(pool), sem(initialValue), pool(pool)
            {}
            ~Submit()
            {
                ;
            }
            static constexpr uint64_t initialValue = 0;
            static constexpr uint64_t signalValue = 1;
            // hold these until completion
            std::vector<std::shared_ptr<Vulkan::Buffer>> buffers; // external
            std::shared_ptr<Vulkan::CommandPool> pool; // external
            Vulkan::CommandBuffer cmd;
            Vulkan::TimelineSemaphore sem;
            std::function<void()> onComplete = nullptr;
        };
        struct SubmitProxy
        {
            Submit& submit;
            Commands& cmds;
            void WaitForCompletion()
            {
                vk::Result res = vk::Result::eTimeout;
                while ((res = Vulkan::Device::Instance().waitSemaphores(vk::SemaphoreWaitInfo({}, *submit.sem, Submit::signalValue), 10'000))
                    != vk::Result::eSuccess)
                {
                    Logger::Error("waitSemaphores returned ", vk::to_string(res),
                        " with ", std::to_string(10'000), "ns timout");
                }
                if (submit.onComplete != nullptr)
                    submit.onComplete();
                for (size_t i = 0; i < cmds.submits.size(); i++)
                {
                    auto& s = cmds.submits[i];
                    if (std::addressof(*s) == std::addressof(submit))
                    {
                        cmds.submits.erase(cmds.submits.begin() + i);
                        return;
                    }
                }
            }
            void OnComplete(std::function<void()> cb)
            {
                submit.onComplete = cb;
            }
        };
        Commands()
        {
            commandPool = std::make_shared<Vulkan::CommandPool>();
            instance = this;
        }
        void WaitAll(std::chrono::nanoseconds pollingTimeout = std::chrono::milliseconds(1))
        {
            if (submits.size() == 0)
                return;

            std::vector<vk::Semaphore> handles;
            handles.reserve(submits.size());
            for (auto& submit : submits)
                handles.push_back(*submit->sem);
            std::vector<uint64_t> waitValues(handles.size(), Submit::signalValue);
            vk::Result res = vk::Result::eTimeout;
            while ((res = Vulkan::Device::Instance().waitSemaphores(vk::SemaphoreWaitInfo({}, handles, waitValues), pollingTimeout.count()))
                != vk::Result::eSuccess)
            {
                Logger::Error("waitSemaphores returned ", vk::to_string(res),
                    " with ", std::to_string(pollingTimeout.count()), "ns timout");
            }
            for (auto& submit : submits)
                if (submit->onComplete != nullptr)
                    submit->onComplete();
            submits.clear();
        }
        SubmitProxy Submit(std::function<void(Vulkan::CommandBuffer&)> fun)
        {
            auto& submit = *submits.emplace_back(std::make_unique<struct Submit>(commandPool));

            submit.cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
            fun(submit.cmd);
            submit.cmd.end();

            auto timelineInfo = vk::TimelineSemaphoreSubmitInfo()
                .setSignalSemaphoreValues(Submit::signalValue);
            Vulkan::Device::Instance().graphicsQueue.submit(vk::SubmitInfo()
                .setCommandBuffers(*submit.cmd)
                .setSignalSemaphores(*submit.sem)
                .setPNext(&timelineInfo)
            );
            return SubmitProxy{ .submit = submit, .cmds = *this };
        }
    public:
        std::vector<std::unique_ptr<struct Submit>> submits;
        std::shared_ptr<Vulkan::CommandPool> commandPool; // dup

        static Commands& Instance()
        {
            return *instance;
        }
    private:
        static Commands* instance;
    };

}