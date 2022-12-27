#pragma once

#include "Buffer.h"
#include "Vulkan/Image.h"
#include "Commands.h"

namespace Nork::Renderer {

    struct BufferCopy
    {
        const void* data;
        vk::DeviceSize size;
        vk::DeviceSize dstOffset;
    };
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

            Commands::Instance().Submit([&](Vulkan::CommandBuffer& cmd)
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
            Commands::Instance().OnRenderFinished([transfer]()
                {
                });
        }
        void UploadToImage(vk::Image img, const void* data, vk::DeviceSize size, uint32_t width, uint32_t height,
            vk::PipelineStageFlagBits2 dstStage, vk::AccessFlagBits2 dstAccess)
        {
            auto transfer = AllocateTransfer(size);
            memcpy(transfer->HostPtr(), data, size);

            Commands::Instance().Submit([&](Vulkan::CommandBuffer& cmd)
                {
                    auto barrier = vk::ImageMemoryBarrier2()
                        .setImage(img)
                        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
                        .setOldLayout(vk::ImageLayout::eUndefined)
                        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                        .setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
                        .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
                        .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite);
                    cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
                 });

            CopyBufferToImage(transfer, img, width, height);

            Commands::Instance().Submit([&](Vulkan::CommandBuffer& cmd)
                {
                    auto barrier = vk::ImageMemoryBarrier2()
                        .setImage(img)
                        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
                        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                        .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                        .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                        .setDstStageMask(dstStage)
                        .setDstAccessMask(dstAccess);
                    cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
                });
        }
    private:
        void CopyBufferToImage(std::shared_ptr<Transfer>& transfer, vk::Image image, uint32_t width, uint32_t height)
        {
            vk::BufferImageCopy region;
            region.bufferOffset = transfer->offset;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = vk::Offset3D(0, 0, 0);
            region.imageExtent = vk::Extent3D(width, height, 1);

            Commands::Instance().Submit([&](Vulkan::CommandBuffer& cmd)
                {
                    cmd.copyBufferToImage(**transfer->buffer->Underlying(), image, vk::ImageLayout::eTransferDstOptimal, region);
                });
            Commands::Instance().OnRenderFinished([transfer]()
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
                    goto found;
                }
                offset = transfer->offset + transfer->size; 
                ++it;
            }
            if (stagingBuffer->memory.Size() - offset >= size)
            {
                goto found;
            }
            std::unreachable();
        found:
            auto transfer = std::make_shared<Transfer>();
            transfer->buffer = stagingBuffer;
            transfer->offset = offset;
            transfer->size = size;
            transfers.insert(it, transfer);
            return transfer;
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