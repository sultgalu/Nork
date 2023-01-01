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
                transferCmds.push_back(std::make_shared<Vulkan::CommandBuffer>(commandPool));
                renderCmds.push_back(std::make_shared<Vulkan::CommandBuffer>(commandPool));
                transferFinishedSemaphores.push_back(std::make_shared<Vulkan::TimelineSemaphore>(1));
                renderFinishedSemaphores.push_back(std::make_shared<Vulkan::TimelineSemaphore>(1));
                renderFinishedSemaphores2.push_back(std::make_shared<Vulkan::BinarySemaphore>());
            }
            renderFinishedCallbacks.resize(maxFramesInFlight);
            transferFinishedCallbacks.resize(maxFramesInFlight);
            instance = this;
        }
        void NextFrame(uint32_t currentFrame)
        {
            this->currentFrame = currentFrame;
        }
        void BeginRenderCommandBuffer()
        {
            Begin(renderCmds[currentFrame], renderFinishedCallbacks[currentFrame], renderFinishedSemaphores[currentFrame]);
        }
        void BeginTransferCommandBuffer()
        {
            Begin(transferCmds[currentFrame], transferFinishedCallbacks[currentFrame], transferFinishedSemaphores[currentFrame]);
        }
        void EndRenderCommandBuffer()
        {
            End(renderCmds[currentFrame]);
        }
        void EndTransferCommandBuffer()
        {
            End(transferCmds[currentFrame]);
        }
        void TransferCommand(std::function<void(Vulkan::CommandBuffer&)> fun)
        {
            fun(*transferCmds[currentFrame]);
        }
        void RenderCommand(std::function<void(Vulkan::CommandBuffer&)> fun)
        {
            fun(*renderCmds[currentFrame]);
        }
        void OnRenderFinished(const std::function<void()>& cb)
        { // TODO: handle out of frame submissions cbs
            renderFinishedCallbacks[currentFrame].push_back(cb);
        }
        void OnTransfersFinished(const std::function<void()>& cb)
        { // TODO: handle out of frame submissions cbs
            transferFinishedCallbacks[currentFrame].push_back(cb);
        }
        void SubmitTransferCommands()
        {
            std::vector<vk::Semaphore> signalSems = {
                **transferFinishedSemaphores[currentFrame]
            };
            std::vector<uint64_t> signalVals = { 1 };
            auto timelineInfo = vk::TimelineSemaphoreSubmitInfo()
                .setSignalSemaphoreValues(signalVals);
            auto submitInfo = vk::SubmitInfo()
                .setSignalSemaphores(signalSems)
                .setCommandBuffers(**transferCmds[currentFrame])
                .setPNext(&timelineInfo);
            Vulkan::Device::Instance().graphicsQueue.submit(submitInfo);
        }
        void SubmitRenderCommands(bool signalBinary = false, const std::vector<vk::Semaphore> waitSems = {})
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
                .setCommandBuffers(**renderCmds[currentFrame])
                .setPNext(&timelineInfo);
            Vulkan::Device::Instance().graphicsQueue.submit(submitInfo);
        }
    private:
        void Begin(std::shared_ptr<Vulkan::CommandBuffer>& cmd, std::vector<std::function<void()>>& cbs,
            std::shared_ptr<Vulkan::TimelineSemaphore>& finishedSem)
        {
            uint64_t waitValue = 1;
            std::chrono::nanoseconds timeout = std::chrono::seconds(10);
            auto res = Vulkan::Device::Instance().waitSemaphores(vk::SemaphoreWaitInfo({}, **finishedSem, waitValue), timeout.count());
            if (res != vk::Result::eSuccess)
                std::unreachable();

            for (auto& cb : cbs)
                cb();
            cbs.clear();
            finishedSem = std::make_shared<Vulkan::TimelineSemaphore>(0);
            cmd->reset();
            cmd->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        }
        void End(std::shared_ptr<Vulkan::CommandBuffer>& cmd)
        {
            cmd->end();
        }
    public:
        std::shared_ptr<Vulkan::CommandPool> commandPool;
        std::vector<std::shared_ptr<Vulkan::CommandBuffer>> renderCmds;
        std::vector<std::shared_ptr<Vulkan::CommandBuffer>> transferCmds;
        std::vector<std::shared_ptr<Vulkan::TimelineSemaphore>> transferFinishedSemaphores;
        std::vector<std::shared_ptr<Vulkan::TimelineSemaphore>> renderFinishedSemaphores;
        std::vector<std::shared_ptr<Vulkan::BinarySemaphore>> renderFinishedSemaphores2;
        std::vector<std::vector<std::function<void()>>> renderFinishedCallbacks;
        std::vector<std::vector<std::function<void()>>> transferFinishedCallbacks;
        uint32_t currentFrame = 0;

        static Commands& Instance()
        {
            return *instance;
        }
    private:
        static Commands* instance;
    };

}