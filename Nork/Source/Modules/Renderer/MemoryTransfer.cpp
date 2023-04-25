#include "MemoryTransfer.h"

namespace Nork::Renderer {
MemoryTransfer::MemoryTransfer()
{
    using enum vk::MemoryPropertyFlagBits;
    stagingBuffer = Buffer::CreateHostVisible(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
        MemoryAllocator::poolSize, MemoryFlags{ .required = eHostVisible | eHostCoherent });
    stagingBuffer->memory.Map();
    instance = this;
}
void MemoryTransfer::DownloadFromBuffer(const Buffer& src, vk::DeviceSize offset, vk::DeviceSize size, const std::function<void(std::span<std::byte>)>& cb) {

    auto transfer = AllocateTransfer(size);

    std::memset(transfer->HostPtr(), 'a', transfer->size); // DEBUG
    Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
    {
        cmd.copyBuffer(**src.Underlying(), **transfer->stagingBuffer->Underlying(),
        vk::BufferCopy(offset, transfer->offset, size));
    });
    Commands::Instance().OnTransfersFinished([transfer, cb]()
    {
        cb(std::span<std::byte>((std::byte*)transfer->HostPtr(), transfer->size));
    });
}
void MemoryTransfer::UploadToBuffer(Buffer& dst, const std::vector<BufferCopy>& copies,
    vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess)
{
    auto transfer = AllocateTransfer(copies);
    std::vector<vk::BufferCopy> vkCopies;
    vk::DeviceSize hostOffset = 0;
    for (auto& copy : copies)
    {
        memcpy((uint8_t*)transfer->HostPtr() + hostOffset, copy.data, copy.size);
        vkCopies.push_back(vk::BufferCopy(transfer->offset + hostOffset, copy.dstOffset, copy.size));
        hostOffset += copy.size;
    }

    Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
    {
        cmd.copyBuffer(**transfer->stagingBuffer->Underlying(), **dst.Underlying(), vkCopies);

        if (syncStages && syncAccess)
        {
            auto barrier = vk::MemoryBarrier2()
                .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
                .setDstStageMask(syncStages)
                .setDstAccessMask(syncAccess);
            // synchronize later memory access on GPU (memory barrier)
            cmd.pipelineBarrier2(vk::DependencyInfo(vk::DependencyFlagBits::eByRegion)
                .setMemoryBarriers(barrier));
        }
    });
    // synchronize later stagingbuffer usage on CPU (timeline sem)
    Commands::Instance().OnTransfersFinished([transfer]()
    {
    });
}
void MemoryTransfer::UploadToImage(vk::Image img, vk::Extent2D extent, vk::Offset2D offset,
    const void* data, vk::DeviceSize size, uint32_t mipLevels,
    vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess,
    vk::ImageLayout finalLayout, vk::ImageLayout initialLayout)
{
    auto transfer = AllocateTransfer(size);
    memcpy(transfer->HostPtr(), data, size);
    bool genMipmaps = mipLevels > 1;

    Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
    {
        auto barrier = vk::ImageMemoryBarrier2()
            .setImage(img)
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1))
            .setOldLayout(initialLayout)
            .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
            .setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
            .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite);
        cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
    });

    CopyBufferToImage(transfer, img, extent, offset);
    if (genMipmaps) { // Create mipmap images
        Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
        { // Set level 0 to transfer read mode
            auto barrier = vk::ImageMemoryBarrier2()
                .setImage(img)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
                .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits2::eTransferRead);
            cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
        });
        CreateMipmaps(img, extent, mipLevels);
        Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
        { // Set level 0 to final layout
            auto barrier = vk::ImageMemoryBarrier2()
                .setImage(img)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
                .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                .setSrcAccessMask(vk::AccessFlagBits2::eTransferRead)
                .setNewLayout(finalLayout)
                .setDstStageMask(dstStage)
                .setDstAccessMask(dstAccess);
            cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
        });
    }

    Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
    { // When genMipmap, the 0. mipLevel was transitioned already
        auto barrier = vk::ImageMemoryBarrier2()
            .setImage(img)
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, genMipmaps ? 1 : 0, genMipmaps ? mipLevels - 1 : 1, 0, 1))
            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
            .setNewLayout(finalLayout)
            .setDstStageMask(dstStage)
            .setDstAccessMask(dstAccess);
        cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
    });
}
void MemoryTransfer::CopyBufferToImage(std::shared_ptr<Transfer>& transfer, vk::Image image, vk::Extent2D extent, vk::Offset2D offset)
{
    vk::BufferImageCopy region;
    region.bufferOffset = transfer->offset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = vk::Offset3D(offset);
    region.imageExtent = vk::Extent3D(extent, 1);

    Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
    {
        cmd.copyBufferToImage(**transfer->stagingBuffer->Underlying(), image, vk::ImageLayout::eTransferDstOptimal, region);
    });
    Commands::Instance().OnTransfersFinished([transfer]()
    {
    });
}
void MemoryTransfer::CreateMipmaps(vk::Image img, vk::Extent2D extent, uint32_t mipLevels) {
    int32_t mipWidth = extent.width;
    int32_t mipHeight = extent.height;

    std::vector<vk::ImageBlit> blits;
    for (uint32_t i = 1; i < mipLevels; i++) {
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
        vk::ImageBlit blit;
        blit.srcOffsets[0] = vk::Offset3D(0);
        blit.srcOffsets[1] = vk::Offset3D(extent.width, extent.height, 1);
        blit.dstOffsets[0] = vk::Offset3D(0);
        blit.dstOffsets[1] = vk::Offset3D(mipWidth, mipHeight, 1); // vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1); // std::max(mipWidth / 2, 1), std::max(mipHeight / 2, 1), 1
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel = 0;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        blits.push_back(blit);
    }

    Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
    {
        cmd.blitImage(img, vk::ImageLayout::eTransferSrcOptimal, img, vk::ImageLayout::eTransferDstOptimal, blits, vk::Filter::eLinear);
    });
}
std::shared_ptr<MemoryTransfer::Transfer> MemoryTransfer::AllocateTransfer(const std::vector<BufferCopy>& copies)
{
    vk::DeviceSize size = 0;
    for (auto& copy : copies)
        size += copy.size;
    return AllocateTransfer(size);
}
std::shared_ptr<MemoryTransfer::Transfer> MemoryTransfer::AllocateTransfer(vk::DeviceSize size)
{
    if (stagingBuffer->memory.Size() < size)
        std::unreachable();

    vk::DeviceSize offset = 0;
    auto it = transfers.begin();
    while (it != transfers.end())
    {
        auto transfer = it->lock();
        if (!transfer)
        {
            it = transfers.erase(it);
            continue;
        }
        auto availableSize = transfer->offset - offset;
        if (availableSize >= size)
        {
            break;
        }
        offset = transfer->offset + transfer->size;
        ++it;
    }
    if (stagingBuffer->memory.Size() - offset < size)
    {
        // not enough space left in stagingbuffer, wait for writes to complete
        FlushTransfers();
        it = transfers.begin();
        offset = 0;
    }

    auto transfer = std::make_shared<Transfer>();
    transfer->stagingBuffer = stagingBuffer;
    transfer->offset = offset;
    transfer->size = size;
    transfers.insert(it, transfer);
    return transfer;
}
void MemoryTransfer::FlushTransfers() // submit, wait for completion, restart with empty stagingBuffer
{
    Commands::Instance().EndTransferCommandBuffer();
    Commands::Instance().SubmitTransferCommands();
    Commands::Instance().BeginTransferCommandBuffer();
    transfers.clear();
}
}