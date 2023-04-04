#pragma once

#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Semaphore.h"

namespace Nork::Renderer {

    class Commands
    {
    private:
        struct CommandBufferExecution {
            std::shared_ptr<Vulkan::CommandBuffer> cmd;
            std::vector<std::function<void()>> onCompletedCallbacks;
            std::shared_ptr<Vulkan::TimelineSemaphore> completedSemaphore;
            bool processed = false;
        };
        struct CBSemaphoreValue {
            static constexpr uint32_t initial = 0;
            static constexpr uint32_t completed = 1;
        };
    public:
        Commands(uint32_t maxFramesInFlight)
        {
            commandPool = std::make_shared<Vulkan::CommandPool>();
            for (size_t i = 0; i < maxFramesInFlight; i++)
            {
                auto makeCmde = [&]() {
                    return CommandBufferExecution {
                    .cmd = std::make_shared<Vulkan::CommandBuffer>(commandPool),
                    .onCompletedCallbacks = {},
                    .completedSemaphore = std::make_shared<Vulkan::TimelineSemaphore>(1)
                    };
                };
                
                transferCmdExecutions.push_back(makeCmde());
                renderCmdExecutions.push_back(makeCmde());
                renderFinishedSemaphores2.push_back(std::make_shared<Vulkan::BinarySemaphore>());
            }
            instance = this;
        }
        void NextFrame(uint32_t currentFrame)
        {
            this->currentFrame = currentFrame;
        }
        void BeginRenderCommandBuffer()
        {
            Begin(renderCmdExecutions[currentFrame]);
        }
        void BeginTransferCommandBuffer()
        {
            Begin(transferCmdExecutions[currentFrame]);
        }
        void EndRenderCommandBuffer()
        {
            renderCmdExecutions[currentFrame].cmd->end();
        }
        void EndTransferCommandBuffer()
        {
            transferCmdExecutions[currentFrame].cmd->end();
        }
        void RenderCommand(std::function<void(Vulkan::CommandBuffer&)> fun)
        {
            fun(*renderCmdExecutions[currentFrame].cmd);
        }
        void TransferCommand(std::function<void(Vulkan::CommandBuffer&)> fun)
        {
            fun(*transferCmdExecutions[currentFrame].cmd);
        }
        void OnRenderFinished(const std::function<void()>& cb)
        { // TODO: handle out of frame submissions cbs
            renderCmdExecutions[currentFrame].onCompletedCallbacks.push_back(cb);
        }
        void OnTransfersFinished(const std::function<void()>& cb)
        { // TODO: handle out of frame submissions cbs
            transferCmdExecutions[currentFrame].onCompletedCallbacks.push_back(cb);
        }
        void SubmitRenderAndTransferCommands(bool signalBinary = false, const std::vector<vk::Semaphore>& waitSems = {})
        {
            GetRenderSubmitInfo(signalBinary, waitSems, [&](auto renderSubmitInfo) {
                GetTransferSubmitInfo([&](auto transferSubmitInfo) {
                    Vulkan::Device::Instance().graphicsQueue.submit({ renderSubmitInfo, transferSubmitInfo });
                });
            });
        }
        void SubmitTransferCommands()
        {
            GetTransferSubmitInfo([](auto submitInfo) {
                Vulkan::Device::Instance().graphicsQueue.submit(submitInfo);
            });
        }
        void SubmitRenderCommands(bool signalBinary = false, const std::vector<vk::Semaphore>& waitSems = {})
        {
            GetRenderSubmitInfo(signalBinary, waitSems, [](auto submitInfo) {
                Vulkan::Device::Instance().graphicsQueue.submit(submitInfo);
            });
        }
        void ProcessFinishedCommandBufferCallbacks() {
            for (auto& cmde : transferCmdExecutions) {
                WaitForSemaphore(cmde, std::chrono::nanoseconds(0));
            }
            for (auto& cmde : renderCmdExecutions) {
                WaitForSemaphore(cmde, std::chrono::nanoseconds(0));
            }
        }
        std::shared_ptr<Vulkan::CommandBuffer> GetCurrentRenderCmd() {
            return renderCmdExecutions[currentFrame].cmd;
        }
        std::shared_ptr<Vulkan::CommandBuffer> GetCurrentTransferCmd() {
            return transferCmdExecutions[currentFrame].cmd;
        }
    private:
        void GetTransferSubmitInfo(std::function<void(const vk::SubmitInfo&)> cb) {
            std::vector<vk::Semaphore> signalSems = {
                **transferCmdExecutions[currentFrame].completedSemaphore
            };
            std::vector<uint64_t> signalVals = { 1 };
            auto timelineInfo = vk::TimelineSemaphoreSubmitInfo()
                .setSignalSemaphoreValues(signalVals);
            cb(vk::SubmitInfo()
                .setSignalSemaphores(signalSems)
                .setCommandBuffers(**transferCmdExecutions[currentFrame].cmd)
                .setPNext(&timelineInfo));
        }
        void GetRenderSubmitInfo(bool signalBinary, const std::vector<vk::Semaphore>& waitSems, std::function<void(vk::SubmitInfo)> cb) {
            vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
            std::vector<vk::Semaphore> signalSems = {
                **renderCmdExecutions[currentFrame].completedSemaphore
            };
            std::vector<uint64_t> signalVals = { 1 };
            if (signalBinary)
            {
                signalSems.push_back(**renderFinishedSemaphores2[currentFrame]);
                signalVals.push_back(0);
            }
            auto timelineInfo = vk::TimelineSemaphoreSubmitInfo()
                .setSignalSemaphoreValues(signalVals);
            cb(vk::SubmitInfo()
                .setWaitDstStageMask(waitStages)
                .setWaitSemaphores(waitSems)
                .setSignalSemaphores(signalSems)
                .setCommandBuffers(**renderCmdExecutions[currentFrame].cmd)
                .setPNext(&timelineInfo));
        }
        vk::Result WaitForSemaphore(CommandBufferExecution& cmde, std::chrono::nanoseconds timeout) {
            if (cmde.processed)
                return vk::Result::eSuccess;

            uint64_t waitValue = 1;
            auto res = Vulkan::Device::Instance().waitSemaphores(vk::SemaphoreWaitInfo({}, **cmde.completedSemaphore, waitValue), timeout.count());
            if (res == vk::Result::eSuccess) {
                for (auto& cb : cmde.onCompletedCallbacks)
                    cb();
                cmde.onCompletedCallbacks.clear();
                cmde.processed = true;
            }
            else {
                ;
            }
            return res;
        }
        void Begin(CommandBufferExecution& cmde)
        {
            if (WaitForSemaphore(cmde, std::chrono::seconds(10)) != vk::Result::eSuccess) {
                std::unreachable(); // should not time out after 10 sec...
            }
            cmde.completedSemaphore = std::make_shared<Vulkan::TimelineSemaphore>(0);
            cmde.processed = false;
            cmde.cmd->reset();
            cmde.cmd->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        }
    public:
        std::shared_ptr<Vulkan::CommandPool> commandPool;
        std::vector<CommandBufferExecution> renderCmdExecutions;
        std::vector<CommandBufferExecution> transferCmdExecutions;
        std::vector<std::shared_ptr<Vulkan::BinarySemaphore>> renderFinishedSemaphores2;
        uint32_t currentFrame = 0;

        static Commands& Instance()
        {
            return *instance;
        }
    private:
        static Commands* instance;
        
        static constexpr uint32_t COMMAND_BUFFER_FINISHED_SEMAPHORE_VALUE = 1;
    };

}