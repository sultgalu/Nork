#pragma once

#include "Buffer.h"
#include "BufferCopy.h"
#include "Vulkan/Image.h"
#include "Commands.h"

namespace Nork::Renderer {
    class MemoryTransfer
    {
    public:
        MemoryTransfer()
        {
            using enum vk::MemoryPropertyFlagBits;
            stagingBuffer = Buffer::CreateHostWritable(vk::BufferUsageFlagBits::eTransferSrc,
                MemoryAllocator::poolSize, MemoryFlags{ .required = eHostVisible | eHostCoherent });
            stagingBuffer->memory.Map();
            instance = this;
        }
        struct Transfer
        {
            std::shared_ptr<HostWritableBuffer> buffer;
            vk::DeviceSize offset;
            vk::DeviceSize size;
            void* HostPtr() { return (uint8_t*)buffer->memory.Ptr() + offset; }
        };
        void UploadToBuffer(Buffer& dst, const std::vector<BufferCopy>& copies, 
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
                cmd.copyBuffer(**transfer->buffer->Underlying(), **dst.Underlying(), vkCopies);

                if (syncStages && syncAccess)
                {
                    auto barrier = vk::MemoryBarrier2()
                        .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                        .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
                        .setDstStageMask(syncStages)
                        .setDstAccessMask(syncAccess);
                    // synchronize later memory access on GPU (memory barrier)
                    cmd.pipelineBarrier2(vk::DependencyInfo(vk::DependencyFlagBits::eByRegion).setMemoryBarriers(barrier));
                }
            });
            // syncrhonize later stagingbuffer usage on CPU (timeline sem)
            Commands::Instance().OnTransfersFinished([transfer]()
                {
                });
        }
        void UploadToImage(vk::Image img, vk::Extent2D extent, vk::Offset2D offset, 
            const void* data, vk::DeviceSize size,
            vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess, 
            vk::ImageLayout finalLayout, vk::ImageLayout initialLayout)
        {
            auto transfer = AllocateTransfer(size);
            memcpy(transfer->HostPtr(), data, size);

            Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
                {
                    auto barrier = vk::ImageMemoryBarrier2()
                        .setImage(img)
                        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
                        .setOldLayout(initialLayout)
                        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                        .setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
                        .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
                        .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite);
                    cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
                 });

            CopyBufferToImage(transfer, img, extent, offset);

            Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
                {
                    auto barrier = vk::ImageMemoryBarrier2()
                        .setImage(img)
                        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
                        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                        .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                        .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
                        .setNewLayout(finalLayout)
                        .setDstStageMask(dstStage)
                        .setDstAccessMask(dstAccess);
                    cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
                });
        }
    private:
        void CopyBufferToImage(std::shared_ptr<Transfer>& transfer, vk::Image image, vk::Extent2D extent, vk::Offset2D offset)
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
                    cmd.copyBufferToImage(**transfer->buffer->Underlying(), image, vk::ImageLayout::eTransferDstOptimal, region);
                });
            Commands::Instance().OnTransfersFinished([transfer]()
                {
                });
        }
        std::shared_ptr<Transfer> AllocateTransfer(const std::vector<BufferCopy>& copies)
        {
            vk::DeviceSize size = 0;
            for (auto& copy : copies)
                size += copy.size;
            return AllocateTransfer(size);
        }
        std::shared_ptr<Transfer> AllocateTransfer(vk::DeviceSize size)
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
            transfer->buffer = stagingBuffer;
            transfer->offset = offset;
            transfer->size = size;
            transfers.insert(it, transfer);
            return transfer;
        }
    private:
        void FlushTransfers() // submit, wait for completion, restart with emtpy stagingBuffer
        {
            Commands::Instance().EndTransferCommandBuffer();
            Commands::Instance().SubmitTransferCommands();
            Commands::Instance().BeginTransferCommandBuffer();
            transfers.clear();
        }
    public:
        std::shared_ptr<HostWritableBuffer> stagingBuffer;
        std::list<std::weak_ptr<Transfer>> transfers;
    public:
        static MemoryTransfer& Instance()
        {
            return *instance;
        }
    private:
        static MemoryTransfer* instance;
    };
}