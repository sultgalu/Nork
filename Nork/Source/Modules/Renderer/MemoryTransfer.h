#pragma once

#include "Buffer.h"
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
                MemoryAllocator::poolSize, eHostVisible | eHostCoherent);
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
        void CopyToBuffer(Buffer& dst, const void* data, vk::DeviceSize size = 0, vk::DeviceSize dstOffset = 0)
        {
            if (size == 0)
                size = dst.memory.Size();
            auto transfer = AllocateTransfer(size);
            memcpy(transfer->HostPtr(), data, size);

            Commands::Instance().Submit([&](Vulkan::CommandBuffer& cmd)
                {
                    cmd.copyBuffer(**transfer->buffer->Underlying(), **dst.Underlying(),
                    vk::BufferCopy(transfer->offset, dstOffset, size));
                })
                .OnComplete([transfer]()
                    {
                    });
        }
        void CopyToImage(vk::Image img, const void* data, vk::DeviceSize size, uint32_t width, uint32_t height)
        {
            auto transfer = AllocateTransfer(size);
            memcpy(transfer->HostPtr(), data, size);

            Commands::Instance().Submit([&](Vulkan::CommandBuffer& cmd)
                {
                    auto barrier = Vulkan::ImageMemoryBarrier(img,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                        {}, vk::AccessFlagBits::eTransferWrite
                        );
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlagBits::eByRegion, {}, {}, { barrier });
                });

            CopyBufferToImage(transfer, img, width, height);

            Commands::Instance().Submit([&](Vulkan::CommandBuffer& cmd)
                {
                    auto barrier = Vulkan::ImageMemoryBarrier(img,
                    vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead
                    );
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlagBits::eByRegion, {}, {}, { barrier });
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
                })
                .OnComplete([transfer]()
                    {
                    });
        }
        std::shared_ptr<Transfer> AllocateTransfer(vk::DeviceSize size)
        {
            vk::DeviceSize offset = 0;
            auto it = transfers.begin();
            for (; it != transfers.end(); ++it)
            {
                auto transfer = it->lock();
                if (!transfer)
                {
                    it = transfers.erase(it);
                    it--;
                    continue;
                }
                auto availableSize = transfer->offset - offset;
                if (availableSize >= size)
                {
                    goto found;
                }
                offset = transfer->offset + transfer->size;
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