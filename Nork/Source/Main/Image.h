#pragma once

#include "DeviceMemory.h"
#include "Sampler.h"

class Image
{
public:
	Image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags memFlags, VkImageAspectFlags aspectFlags, std::shared_ptr<Sampler> sampler = nullptr)
	    : sampler(sampler)
    {
        imageInfo = VkImageCreateInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        layout = imageInfo.initialLayout;

        vkCreateImage(Device::Instance().device, &imageInfo, nullptr, &handle) == VkSuccess();

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

        vkCreateImageView(Device::Instance().device, &viewInfo, nullptr, &imageView) == VkSuccess();
	}
    ~Image()
    {
        vkDestroyImage(Device::Instance().device, handle, nullptr);
    }
    void Sampler(std::shared_ptr<Sampler> sampler)
    {
        this->sampler = sampler;
    }
public:
    VkImage handle;
    VkImageView imageView;
    std::shared_ptr<class Sampler> sampler = nullptr;

    VkImageCreateInfo imageInfo;
    VkImageLayout layout;

    std::shared_ptr<DeviceMemory> memory;
    VkDeviceSize memOffset;
};