#pragma once

#include "Device.h"

class CommandBuilder;
class CommandBuffer
{
public:
    CommandBuffer(VkCommandBuffer handle)
        : handle(handle)
    {}
    ~CommandBuffer() = default;
    CommandBuilder CommandBuilder();
public:
    VkCommandBuffer handle;
};
class CommandPool
{
public:
    CommandPool()
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = Device::Instance().graphicsQueueFamily;

        vkCreateCommandPool(Device::Instance().device, &poolInfo, nullptr, &handle) == VkSuccess();
    }
    ~CommandPool()
    {
        vkDestroyCommandPool(Device::Instance().device, handle, nullptr);
    }
    std::vector<CommandBuffer> CreateCommandBuffers(uint32_t count)
    {
        std::vector<VkCommandBuffer> cmdBufHandles(count);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = handle;
        allocInfo.commandBufferCount = cmdBufHandles.size();
        vkAllocateCommandBuffers(Device::Instance().device, &allocInfo, cmdBufHandles.data()) == VkSuccess();
        std::vector<CommandBuffer> cmdBuffers;
        cmdBuffers.reserve(count);
        for (auto& handle : cmdBufHandles)
            cmdBuffers.emplace_back(handle);
        return cmdBuffers;
    }
    CommandBuffer CreateCommandBuffer()
    {
        return CreateCommandBuffers(1)[0];
    }
    void FreeCommandBuffers(std::vector<CommandBuffer> cmdBufs)
    {
        std::vector<VkCommandBuffer> handles;
        handles.reserve(cmdBufs.size());
        for (auto& cmdBuf : cmdBufs)
            handles.push_back(cmdBuf.handle);
        vkFreeCommandBuffers(Device::Instance().device, handle, handles.size(), handles.data());
    }
    void FreeCommandBuffer(CommandBuffer cmdBuf)
    {
        vkFreeCommandBuffers(Device::Instance().device, handle, 1, &cmdBuf.handle);
    }
public:
    VkCommandPool handle;
};

class RenderPassBuilder
{
public:
    using Self = RenderPassBuilder;
    RenderPassBuilder(CommandBuilder& cmdBuilder, const SwapChain& swapChain);
    Self& Begin(uint32_t imgIdx)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain.renderPass;
        renderPassInfo.framebuffer = swapChain.swapChainFramebuffers[imgIdx];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChain.swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(cmdBuf.handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        return *this;
    }
    Self& BindPipeline(const Pipeline& pipeline)
    {
        vkCmdBindPipeline(cmdBuf.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
        return *this;
    }
    Self& BindDescriptorSet(VkPipelineLayout layoutHandle, DescriptorSet& descriptorSet)
    {
        vkCmdBindDescriptorSets(cmdBuf.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, layoutHandle, 0, 1, &descriptorSet.handle, 0, nullptr);
        return *this;
    }
    Self& BindVB(const Buffer& buffer)
    {
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmdBuf.handle, 0, 1, &buffer.handle, offsets);
        return *this;
    }
    Self& BindIB(const Buffer& buffer, VkIndexType type)
    {
        vkCmdBindIndexBuffer(cmdBuf.handle, buffer.handle, 0, type);
        return *this;
    }
    Self& Draw(uint32_t count)
    {
        vkCmdDraw(cmdBuf.handle, count, 1, 0, 0);
        return *this;
    }
    Self& DrawIndexed(uint32_t count, uint32_t instanceCount = 1)
    {
        vkCmdDrawIndexed(cmdBuf.handle, count, instanceCount, 0, 0, 0);
        return *this;
    }
    Self& DrawIndexedIndirect(const Buffer& buffer, uint32_t drawCount, VkDeviceSize offset = 0, uint32_t stride = sizeof(VkDrawIndexedIndirectCommand))
    {
        vkCmdDrawIndexedIndirect(cmdBuf.handle, buffer.handle, offset, drawCount, stride);
        return *this;
    }
    Self& Viewport(float x, float y, float width, float height)
    {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = width;
        viewport.height = height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmdBuf.handle, 0, 1, &viewport);
        viewportSet = true;
        return *this;
    }
    Self& Scissor(int32_t offsX, int32_t offsY, uint32_t width, uint32_t height)
    {
        VkRect2D scissor{};
        scissor.offset = { offsX, offsY };
        scissor.extent = { width, height }; // swapChain.swapChainExtent;
        vkCmdSetScissor(cmdBuf.handle, 0, 1, &scissor);
        scissorSet = true;
        return *this;
    }
    Self& Viewport()
    {
        return Viewport(0, 0, swapChain.swapChainExtent.width, swapChain.swapChainExtent.height);
    }
    Self& Scissor()
    {
        return Scissor(0, 0, swapChain.swapChainExtent.width, swapChain.swapChainExtent.height);
    }
    CommandBuilder& EndRenderPass()
    {
        vkCmdEndRenderPass(cmdBuf.handle);
        return cmdBuilder;
    }
public:
    CommandBuilder& cmdBuilder;
    CommandBuffer& cmdBuf;
    const SwapChain& swapChain;
    bool viewportSet = false;
    bool scissorSet = false;
};

class CommandBuilder
{
public:
    using Self = CommandBuilder;
    CommandBuilder(CommandBuffer& cmdBuf)
        : cmdBuf(cmdBuf)
    {}
    CommandBuilder(CommandPool& pool)
        : cmdBuf(pool.CreateCommandBuffer())
    {}
    Self& BeginCommands(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = flags; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional
        vkBeginCommandBuffer(cmdBuf.handle, &beginInfo) == VkSuccess();
        return *this;
    }
    CommandBuffer EndCommands()
    {
        vkEndCommandBuffer(cmdBuf.handle) == VkSuccess();
        return cmdBuf;
    }
    RenderPassBuilder BeginRenderPass(const SwapChain& swapChain, uint32_t imgIdx)
    {
        return RenderPassBuilder(*this, swapChain)
            .Begin(imgIdx);
    }
    Self& PipelineBarrier(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImageMemoryBarrier barrier)
    {
        vkCmdPipelineBarrier(cmdBuf.handle, srcStage, dstStage, 0,0, nullptr, 0, nullptr, 1, &barrier);
        return *this;
    }
    Self& CopyBufferToImage(const Buffer& buffer, const Image& img, VkBufferImageCopy region)
    {
        vkCmdCopyBufferToImage(cmdBuf.handle, buffer.handle , img.handle, img.layout, 1, &region);
        return *this;
    }
    Self& CopyBuffer(const Buffer& src, const Buffer& dst, VkDeviceSize size, VkDeviceSize srcOffs = 0, VkDeviceSize dstOffs = 0)
    {
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        copyRegion.srcOffset = srcOffs;
        copyRegion.dstOffset = dstOffs;
        vkCmdCopyBuffer(cmdBuf.handle, src.handle, dst.handle, 1, &copyRegion);
        return *this;
    }
public:
    CommandBuffer cmdBuf;
};

using CommandBuilder_ = CommandBuilder;
RenderPassBuilder::RenderPassBuilder(CommandBuilder& cmdBuilder, const SwapChain& swapChain)
    : cmdBuilder(cmdBuilder), cmdBuf(cmdBuilder.cmdBuf), swapChain(swapChain)
{}
CommandBuilder_ CommandBuffer::CommandBuilder()
{
    return CommandBuilder_(*this);
}