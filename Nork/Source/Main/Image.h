#pragma once

#include "DeviceMemory.h"
#include "Sampler.h"

class Image
{
public:
    Image(const Image&) = delete;
    Image() = default;
    VkImageMemoryBarrier Barrier(VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = handle;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = srcAccess;
        barrier.dstAccessMask = dstAccess;
        return barrier;
    }
    virtual uint32_t Width() const = 0;
    virtual uint32_t Height() const = 0;
    virtual VkFormat Format() const = 0;
    virtual VkImageView ImageView() const = 0;
    virtual std::shared_ptr<Sampler> Sampler() const = 0;
public:
    VkImage handle;

};
class SwapChainImage : public Image
{
public:
    SwapChainImage(VkImage handle, const VkSwapchainCreateInfoKHR& createInfo)
    {
        this->handle = handle;
        this->createInfo = createInfo;
    }
    virtual uint32_t Width() const { return createInfo.imageExtent.width; }
    virtual uint32_t Height() const { return createInfo.imageExtent.height; }
    virtual VkFormat Format() const { return createInfo.imageFormat; }
    virtual VkImageView ImageView() const
    {
        std::unreachable();
    }
    virtual std::shared_ptr<class Sampler> Sampler() const
    {
        std::unreachable();
    }
public:
    VkSwapchainCreateInfoKHR createInfo;
};
class AppImage : public Image
{
public:
    AppImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags memFlags, VkImageAspectFlags aspectFlags, std::shared_ptr<class Sampler> sampler = nullptr)
        : sampler(sampler)
    {
        createInfo = VkImageCreateInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.extent.width = width;
        createInfo.extent.height = height;
        createInfo.extent.depth = 1;
        createInfo.mipLevels = 1;
        createInfo.arrayLayers = 1;
        createInfo.format = format;
        createInfo.tiling = tiling;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        createInfo.usage = usage;
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateImage(Device::Instance().device, &createInfo, nullptr, &handle) == VkSuccess();

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(Device::Instance().device, handle, &memRequirements);
        memory = std::make_shared<DeviceMemory>(memRequirements.size, memRequirements.memoryTypeBits, memFlags);

        vkBindImageMemory(Device::Instance().device, handle, memory->handle, 0);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = handle;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(Device::Instance().device, &viewInfo, nullptr, &imageViewHandle) == VkSuccess();
    }
    ~AppImage()
    {
        vkDestroyImageView(Device::Instance().device, imageViewHandle, nullptr);
        vkDestroyImage(Device::Instance().device, handle, nullptr);
    }
    virtual uint32_t Width() const { return createInfo.extent.width; }
    virtual uint32_t Height() const { return createInfo.extent.height; }
    virtual VkFormat Format() const { return createInfo.format; }
    virtual VkImageView ImageView() const
    {
        return imageViewHandle;
    }
    virtual std::shared_ptr<class Sampler> Sampler() const
    {
        return sampler;
    }
public:
    VkImageView imageViewHandle;
    std::shared_ptr<class Sampler> sampler;
    VkImageCreateInfo createInfo;

    std::shared_ptr<DeviceMemory> memory;
    VkDeviceSize memOffset;
};