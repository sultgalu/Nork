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
        Commands(uint32_t maxFramesInFlight);
        void NextFrame(uint32_t currentFrame);
        void BeginRenderCommandBuffer();
        void BeginTransferCommandBuffer();
        void EndRenderCommandBuffer();
        void EndTransferCommandBuffer();
        void RenderCommand(std::function<void(Vulkan::CommandBuffer&)> fun);
        void TransferCommand(std::function<void(Vulkan::CommandBuffer&)> fun);
        void OnRenderFinished(const std::function<void()>& cb);
        void OnTransfersFinished(const std::function<void()>& cb);
        void SubmitRenderAndTransferCommands(bool signalBinary = false, const std::vector<vk::Semaphore>& waitSems = {});
        void SubmitTransferCommands();
        void SubmitRenderCommands(bool signalBinary = false, const std::vector<vk::Semaphore>& waitSems = {});
        void ProcessFinishedCommandBufferCallbacks();
        std::shared_ptr<Vulkan::CommandBuffer> GetCurrentRenderCmd();
        std::shared_ptr<Vulkan::CommandBuffer> GetCurrentTransferCmd();
    private:
        void GetTransferSubmitInfo(std::function<void(const vk::SubmitInfo&)> cb);
        void GetRenderSubmitInfo(bool signalBinary, const std::vector<vk::Semaphore>& waitSems, std::function<void(vk::SubmitInfo)> cb);
        vk::Result WaitForSemaphore(CommandBufferExecution& cmde, std::chrono::nanoseconds timeout);
        void Begin(CommandBufferExecution& cmde);
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