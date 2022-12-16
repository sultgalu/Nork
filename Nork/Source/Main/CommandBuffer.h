#pragma once

#include "SwapChain.h"
#include "Pipeline.h"
#include "DescriptorSet.h"
#include "Framebuffer.h"

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
    RenderPassBuilder(CommandBuilder& cmdBuilder, const Framebuffer& fb);

    Self& Begin(const RenderPass& renderPass)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass.handle;
        renderPassInfo.framebuffer = fb.handle;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { fb.width, fb.height };

        std::vector<VkClearValue> clearValues;
        clearValues.reserve(fb.renderPassConfig.attachments.size());
        for (auto& att : fb.renderPassConfig.attachments)
        {
            VkClearValue clearValue{};
            if (att.format == VK_FORMAT_D32_SFLOAT)
                clearValue.depthStencil = { 1.0f, 0 };
            else if (att.format == VK_FORMAT_R8G8B8A8_SRGB)
                clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            else
                std::unreachable();
            clearValues.push_back(clearValue);
        }

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(cmdBuf.handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        return *this;
    }
    Self& BindPipeline(const Pipeline& pipeline)
    {
        vkCmdBindPipeline(cmdBuf.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
        return *this;
    }
    Self& BindDescriptorSet(VkPipelineLayout layoutHandle, DescriptorSet& descriptorSet, const std::vector<uint32_t>& dynamicOffsets = {})
    {
        const uint32_t* offsets = dynamicOffsets.size() == 0 ? nullptr : dynamicOffsets.data();
        vkCmdBindDescriptorSets(cmdBuf.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, layoutHandle, 0, 1, &descriptorSet.handle,
            dynamicOffsets.size(), offsets);
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
    Self& PushConstants(const Pipeline& pipeline, VkShaderStageFlags stages, const void* data, uint32_t size, uint32_t offs = 0)
    {
        vkCmdPushConstants(cmdBuf.handle, pipeline.layoutHandle, stages, offs, size, data);
        return *this;
    }
    Self& Draw(uint32_t count)
    {
        vkCmdDraw(cmdBuf.handle, count, 1, 0, 0);
        return *this;
    }
    Self& DrawQuad()
    {
        return Draw(6);
    }
    Self& DrawIndexed(uint32_t count, uint32_t instanceCount = 1)
    {
        vkCmdDrawIndexed(cmdBuf.handle, count, instanceCount, 0, 0, 0);
        return *this;
    }
    Self& DrawIndexedIndirect(const Buffer& buffer, uint32_t count, VkDeviceSize offset = 0, uint32_t stride = sizeof(VkDrawIndexedIndirectCommand))
    {
        vkCmdDrawIndexedIndirect(cmdBuf.handle, buffer.handle, offset, count, stride);
        return *this;
    }
    Self& NextSubPass()
    {
        vkCmdNextSubpass(cmdBuf.handle, VK_SUBPASS_CONTENTS_INLINE);
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
        return Viewport(0, 0, fb.width, fb.height);
    }
    Self& Scissor()
    {
        return Scissor(0, 0, fb.width, fb.height);
    }
    CommandBuilder& EndRenderPass()
    {
        vkCmdEndRenderPass(cmdBuf.handle);
        return cmdBuilder;
    }
public:
    CommandBuilder& cmdBuilder;
    CommandBuffer& cmdBuf;
    const Framebuffer& fb;
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
    RenderPassBuilder BeginRenderPass(const Framebuffer& fb, const RenderPass& renderPass)
    {
        return RenderPassBuilder(*this, fb)
            .Begin(renderPass);
    }
    Self& PipelineBarrier(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::vector<VkImageMemoryBarrier> barriers)
    {
        vkCmdPipelineBarrier(cmdBuf.handle, srcStage, dstStage, 0,0, nullptr, 0, nullptr, barriers.size(), barriers.data());
        return *this;
    }
    Self& CopyBufferToImage(const Buffer& buffer, const Image& img, VkBufferImageCopy region, VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        vkCmdCopyBufferToImage(cmdBuf.handle, buffer.handle , img.handle, layout, 1, &region);
        return *this;
    }
    Self& CopyImage(const Image& src, VkImageLayout srcLayout, VkImageAspectFlags srcAspect, const Image& dst, VkImageLayout dstLayout, VkImageAspectFlags dstAspect, VkExtent3D extent)
    {
        VkImageCopy copy{};
        copy.dstSubresource.aspectMask = dstAspect;
        copy.dstSubresource.layerCount = 1;
        copy.dstSubresource.baseArrayLayer = 0;
        copy.dstSubresource.mipLevel = 0;

        copy.srcSubresource.aspectMask = srcAspect;
        copy.srcSubresource.layerCount = 1;
        copy.srcSubresource.baseArrayLayer = 0;
        copy.srcSubresource.mipLevel = 0;

        copy.srcOffset = VkOffset3D{ 0, 0, 0 };
        copy.dstOffset = VkOffset3D{ 0, 0, 0 };
        copy.extent = extent;
        vkCmdCopyImage(cmdBuf.handle, src.handle, srcLayout, dst.handle, dstLayout, 1, &copy);
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
RenderPassBuilder::RenderPassBuilder(CommandBuilder& cmdBuilder, const Framebuffer& fb)
    : cmdBuilder(cmdBuilder), cmdBuf(cmdBuilder.cmdBuf), fb(fb)
{}
CommandBuilder_ CommandBuffer::CommandBuilder()
{
    return CommandBuilder_(*this);
}