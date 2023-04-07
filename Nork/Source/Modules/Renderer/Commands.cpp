#include "Commands.h"

namespace Nork::Renderer {
Commands::Commands(uint32_t maxFramesInFlight)
{
    commandPool = std::make_shared<Vulkan::CommandPool>();
    for (size_t i = 0; i < maxFramesInFlight; i++)
    {
        auto makeCmde = [&]() {
            return CommandBufferExecution{
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
void Commands::NextFrame(uint32_t currentFrame)
{
    this->currentFrame = currentFrame;
}
void Commands::BeginRenderCommandBuffer()
{
    Begin(renderCmdExecutions[currentFrame]);
}
void Commands::BeginTransferCommandBuffer()
{
    Begin(transferCmdExecutions[currentFrame]);
}
void Commands::EndRenderCommandBuffer()
{
    renderCmdExecutions[currentFrame].cmd->end();
}
void Commands::EndTransferCommandBuffer()
{
    transferCmdExecutions[currentFrame].cmd->end();
}
void Commands::RenderCommand(std::function<void(Vulkan::CommandBuffer&)> fun)
{
    fun(*renderCmdExecutions[currentFrame].cmd);
}
void Commands::TransferCommand(std::function<void(Vulkan::CommandBuffer&)> fun)
{
    fun(*transferCmdExecutions[currentFrame].cmd);
}
void Commands::OnRenderFinished(const std::function<void()>& cb)
{ // TODO: handle out of frame submissions cbs
    renderCmdExecutions[currentFrame].onCompletedCallbacks.push_back(cb);
}
void Commands::OnTransfersFinished(const std::function<void()>& cb)
{ // TODO: handle out of frame submissions cbs
    transferCmdExecutions[currentFrame].onCompletedCallbacks.push_back(cb);
}
void Commands::SubmitRenderAndTransferCommands(bool signalBinary, const std::vector<vk::Semaphore>& waitSems)
{
    GetRenderSubmitInfo(signalBinary, waitSems, [&](auto renderSubmitInfo) {
        GetTransferSubmitInfo([&](auto transferSubmitInfo) {
            Vulkan::Device::Instance().graphicsQueue.submit({ transferSubmitInfo, renderSubmitInfo });
        });
    });
}
void Commands::SubmitTransferCommands()
{
    GetTransferSubmitInfo([](auto submitInfo) {
        Vulkan::Device::Instance().graphicsQueue.submit(submitInfo);
    });
}
void Commands::SubmitRenderCommands(bool signalBinary, const std::vector<vk::Semaphore>& waitSems)
{
    GetRenderSubmitInfo(signalBinary, waitSems, [](auto submitInfo) {
        Vulkan::Device::Instance().graphicsQueue.submit(submitInfo);
    });
}
void Commands::ProcessFinishedCommandBufferCallbacks() {
    for (auto& cmde : transferCmdExecutions) {
        WaitForSemaphore(cmde, std::chrono::nanoseconds(0));
    }
    for (auto& cmde : renderCmdExecutions) {
        WaitForSemaphore(cmde, std::chrono::nanoseconds(0));
    }
}
std::shared_ptr<Vulkan::CommandBuffer> Commands::GetCurrentRenderCmd() {
    return renderCmdExecutions[currentFrame].cmd;
}
std::shared_ptr<Vulkan::CommandBuffer> Commands::GetCurrentTransferCmd() {
    return transferCmdExecutions[currentFrame].cmd;
}
void Commands::GetTransferSubmitInfo(std::function<void(const vk::SubmitInfo&)> cb) {
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
void Commands::GetRenderSubmitInfo(bool signalBinary, const std::vector<vk::Semaphore>& waitSems, std::function<void(vk::SubmitInfo)> cb) {
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
vk::Result Commands::WaitForSemaphore(CommandBufferExecution& cmde, std::chrono::nanoseconds timeout) {
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
    return res;
}
void Commands::Begin(CommandBufferExecution& cmde)
{
    if (WaitForSemaphore(cmde, std::chrono::seconds(10)) != vk::Result::eSuccess) {
        std::unreachable(); // should not time out after 10 sec...
    }
    cmde.completedSemaphore = std::make_shared<Vulkan::TimelineSemaphore>(0);
    cmde.processed = false;
    cmde.cmd->reset();
    cmde.cmd->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
}
}