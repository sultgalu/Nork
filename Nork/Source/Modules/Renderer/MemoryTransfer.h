#pragma once

#include "Buffer.h"
#include "BufferCopy.h"
#include "Vulkan/Image.h"
#include "Commands.h"

namespace Nork::Renderer {
    class MemoryTransfer
    {
    public:
        struct ImageBarrierInfo { // barrier called 2X: before(src used) and after(dst used) operation
            vk::PipelineStageFlags2 srcStage, dstStage;
            vk::AccessFlags2 srcAccess, dstAccess;
            vk::ImageLayout initialLayout, finalLayout;
        };

        MemoryTransfer();
        struct Transfer
        {
            std::shared_ptr<HostVisibleBuffer> stagingBuffer;
            vk::DeviceSize offset;
            vk::DeviceSize size;
            void* HostPtr() { return (uint8_t*)stagingBuffer->memory.Ptr() + offset; }
        };
        void DownloadFromBuffer(const Buffer& src, vk::DeviceSize offset, vk::DeviceSize size, const std::function<void(std::span<std::byte>)>& cb);
        void UploadToBuffer(Buffer& dst, const std::vector<BufferCopy>& copies,
            vk::PipelineStageFlags2 syncStages, vk::AccessFlags2 syncAccess);
        void UploadToImage(vk::Image img, vk::Extent2D extent, vk::Offset2D offset,
            const void* data, vk::DeviceSize size, uint32_t mipLevels,
            vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess,
            vk::ImageLayout finalLayout, vk::ImageLayout initialLayout);
        void CopyImage(vk::Image src, vk::Image dst, ImageBarrierInfo srcBarrierInfo, ImageBarrierInfo dstBarrierInfo, 
            vk::Extent3D extent, vk::Offset3D scrOffs = {}, vk::Offset3D dstOffs = {}, vk::ImageSubresourceLayers = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
    private:
        void CopyBufferToImage(std::shared_ptr<Transfer>& transfer, vk::Image image, vk::Extent2D extent, vk::Offset2D offset);
        void CreateMipmaps(vk::Image img, vk::Extent2D extent, uint32_t mipLevels);
        std::shared_ptr<Transfer> AllocateTransfer(const std::vector<BufferCopy>& copies);
        std::shared_ptr<Transfer> AllocateTransfer(vk::DeviceSize size);
    private:
        void FlushTransfers();
    public:
        std::shared_ptr<HostVisibleBuffer> stagingBuffer;
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