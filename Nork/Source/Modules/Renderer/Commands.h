#pragma once

#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Semaphore.h"

namespace Nork::Renderer {

    class Commands
    {
    public:
        Commands(uint32_t maxFramesInFlight)
        {
            commandPool = std::make_shared<Vulkan::CommandPool>();
            for (size_t i = 0; i < maxFramesInFlight; i++)
            {
                cmds.push_back(std::make_shared<Vulkan::CommandBuffer>(commandPool));
                renderFinishedSemaphores.push_back(std::make_shared<Vulkan::TimelineSemaphore>(1));
                renderFinishedSemaphores2.push_back(std::make_shared<Vulkan::BinarySemaphore>());
            }
            renderFinishedCallbacks.resize(maxFramesInFlight);
            instance = this;
        }
        void Begin(uint32_t currentFrame)
        {
            this->currentFrame = currentFrame;

            uint64_t waitValue = 1;
            std::chrono::nanoseconds timeout = std::chrono::seconds(1);
            auto res = Vulkan::Device::Instance().waitSemaphores(vk::SemaphoreWaitInfo({}, **renderFinishedSemaphores[currentFrame], waitValue), timeout.count());
            if (res != vk::Result::eSuccess)
                std::unreachable();
            
            for (auto& cb : renderFinishedCallbacks[currentFrame])
                cb();
            renderFinishedCallbacks[currentFrame].clear();
            renderFinishedSemaphores[currentFrame] = std::make_shared<Vulkan::TimelineSemaphore>(0);
            cmds[currentFrame]->reset();
            cmds[currentFrame]->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        }
        void End()
        {
            cmds[currentFrame]->end();
        }
        void Submit(std::function<void(Vulkan::CommandBuffer&)> fun)
        {
            // auto& submit = *submits.emplace_back(std::make_unique<struct Submit>(commandPool));

            fun(*cmds[currentFrame]);

            // Vulkan::Device::Instance().graphicsQueue.submit(vk::SubmitInfo().setCommandBuffers(*submit.cmd));
        }
        void OnRenderFinished(const std::function<void()>& cb)
        {
            renderFinishedCallbacks[currentFrame].push_back(cb);
        }
        void Submit(bool signalBinary = false, const std::vector<vk::Semaphore> waitSems = {})
        {
            vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
            std::vector<vk::Semaphore> signalSems = { 
                **renderFinishedSemaphores[currentFrame]
            };
            std::vector<uint64_t> signalVals = { 1 };
            if (signalBinary)
            {
                signalSems.push_back(**renderFinishedSemaphores2[currentFrame]);
                signalVals.push_back(0);
            }
            auto timelineInfo = vk::TimelineSemaphoreSubmitInfo()
                .setSignalSemaphoreValues(signalVals);
            auto submitInfo = vk::SubmitInfo()
                .setWaitDstStageMask(waitStages)
                .setWaitSemaphores(waitSems)
                .setSignalSemaphores(signalSems)
                .setCommandBuffers(**cmds[currentFrame])
                .setPNext(&timelineInfo);
            Vulkan::Device::Instance().graphicsQueue.submit(submitInfo);
        }
    public:
        std::shared_ptr<Vulkan::CommandPool> commandPool;
        std::vector<std::shared_ptr<Vulkan::CommandBuffer>> cmds;
        std::vector<std::shared_ptr<Vulkan::TimelineSemaphore>> renderFinishedSemaphores;
        std::vector<std::shared_ptr<Vulkan::BinarySemaphore>> renderFinishedSemaphores2;
        std::vector<std::vector<std::function<void()>>> renderFinishedCallbacks;
        uint32_t currentFrame = 0;

        static Commands& Instance()
        {
            return *instance;
        }
    private:
        static Commands* instance;
    };

}