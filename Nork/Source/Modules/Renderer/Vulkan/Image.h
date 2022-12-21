#pragma once

#include "DeviceMemory.h"
#include "Sampler.h"

namespace Nork::Renderer::Vulkan {
    struct Format
    {
        constexpr static auto rgba8Unorm = vk::Format::eR8G8B8A8Unorm;
        constexpr static auto rgba8Ui = vk::Format::eR8G8B8A8Uint;
        constexpr static auto rgba8Srgb = vk::Format::eR8G8B8A8Srgb;
        constexpr static auto rgba16f = vk::Format::eR16G16B16A16Sfloat;
        constexpr static auto rgba32f = vk::Format::eR32G32B32A32Sfloat;
        constexpr static auto depth32 = vk::Format::eD32Sfloat;
        constexpr static auto depth16 = vk::Format::eD16Unorm;
    };

    struct ImageMemoryBarrier : vk::ImageMemoryBarrier
    {
        ImageMemoryBarrier(vk::Image img, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
            vk::AccessFlags srcAccess, vk::AccessFlags dstAccess, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor)
        {
            this->oldLayout = oldLayout;
            this->newLayout = newLayout;
            this->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            this->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            this->image = img;
            this->subresourceRange.aspectMask = aspect;
            this->subresourceRange.baseMipLevel = 0;
            this->subresourceRange.levelCount = 1;
            this->subresourceRange.baseArrayLayer = 0;
            this->subresourceRange.layerCount = 1;
            this->srcAccessMask = srcAccess;
            this->dstAccessMask = dstAccess;
        }
    };

    class ImageBase : public vk::raii::Image
    {
    public:
        using vk::raii::Image::Image;
        ~ImageBase()
        {
            ;
        }
        virtual uint32_t Width() const = 0;
        virtual uint32_t Height() const = 0;
        virtual vk::Format Format() const = 0;
    };
    struct ImageCreateInfo : vk::ImageCreateInfo
    {
        ImageCreateInfo() = delete;
        ImageCreateInfo(uint32_t width, uint32_t height, vk::Format format_, vk::ImageUsageFlags usage_)
        {
            imageType = vk::ImageType::e2D;
            extent.width = width;
            extent.height = height;
            extent.depth = 1;
            mipLevels = 1;
            arrayLayers = 1;
            format = format_;
            tiling = vk::ImageTiling::eOptimal;
            initialLayout = vk::ImageLayout::eUndefined;
            usage = usage_;
            samples = vk::SampleCountFlagBits::e1;
            sharingMode = vk::SharingMode::eExclusive;
        }
    };
    class Image : public ImageBase
    {
    public:
        Image(const ImageCreateInfo& createInfo, vk::MemoryPropertyFlags memFlags)
            : ImageBase(Device::Instance(), createInfo), createInfo(createInfo)
        {
            auto memreq = this->getMemoryRequirements();
            VkMemoryAllocateInfo i;
            memory = std::make_shared<DeviceMemory>(vk::MemoryAllocateInfo(memreq.size,
                Device::Instance().findMemoryType(memreq.memoryTypeBits, memFlags)));

            bindMemory(**memory, 0);
        }
        uint32_t Width() const override { return createInfo.extent.width; }
        uint32_t Height() const override { return createInfo.extent.height; }
        vk::Format Format() const override { return createInfo.format; }
    public:
        ImageCreateInfo createInfo;
        std::shared_ptr<DeviceMemory> memory;
    };

    struct ImageViewCreateInfo : vk::ImageViewCreateInfo
    {
        ImageViewCreateInfo() = delete;
        ImageViewCreateInfo(std::shared_ptr<Image> img_, vk::ImageAspectFlagBits aspect, vk::ImageViewType type = vk::ImageViewType::e2D)
            : ImageViewCreateInfo(**img_, img_->Format(), aspect, type)
        {
            img = img_;
        }
        ImageViewCreateInfo(vk::Image img, vk::Format format_, vk::ImageAspectFlagBits aspect, vk::ImageViewType type = vk::ImageViewType::e2D)
        {
            image = img;
            viewType = type;
            format = format_;
            subresourceRange.aspectMask = aspect;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;
        }
        std::shared_ptr<Image> img = nullptr;
    };

    class ImageView : public vk::raii::ImageView
    {
    public:
        ImageView(const ImageViewCreateInfo& createInfo, std::shared_ptr<class Sampler> sampler = nullptr)
            : createInfo(createInfo), vk::raii::ImageView(Device::Instance(), createInfo), sampler(sampler)
        {}
        std::shared_ptr<Image> Image()
        {
            return createInfo.img;
        }
    public:
        ImageViewCreateInfo createInfo;
        std::shared_ptr<class Sampler> sampler;
    };
}