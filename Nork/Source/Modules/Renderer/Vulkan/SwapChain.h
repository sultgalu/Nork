#pragma once
#include "Device.h"

namespace Nork::Renderer::Vulkan {
    struct SwapChainCreateInfo : vk::SwapchainCreateInfoKHR
    {
        SwapChainCreateInfo(vk::SurfaceKHR surface, const vk::SurfaceCapabilitiesKHR& capabilities,
            uint32_t imgCount, VkExtent2D extent)
        {
            // uint32_t imageCount = 3; // triple buffering //capabilities.minImageCount + 1;
            if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < imgCount)
            {
                std::unreachable();
                //imgCount = capabilities.maxImageCount;
            }
            this->surface = surface;
            this->minImageCount = imgCount;
            auto format = chooseSwapSurfaceFormat();
            this->imageFormat = format.format;
            this->imageColorSpace = format.colorSpace;
            this->imageExtent = extent;
            this->imageArrayLayers = 1;
            this->imageUsage = vk::ImageUsageFlagBits::eTransferDst; // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT use VK_IMAGE_USAGE_TRANSFER_DST_BIT if it is not a destination (we need this)

            if (PhysicalDevice::Instance().graphicsQueueFamily != PhysicalDevice::Instance().presentQueueFamily)
            {
                uint32_t queueFamilyIndices[] = { PhysicalDevice::Instance().graphicsQueueFamily, PhysicalDevice::Instance().presentQueueFamily };
                this->imageSharingMode = vk::SharingMode::eConcurrent;
                this->queueFamilyIndexCount = 2;
                this->pQueueFamilyIndices = queueFamilyIndices;
            }
            else
            {
                this->imageSharingMode = vk::SharingMode::eExclusive;
                this->queueFamilyIndexCount = 0; // Optional
                this->pQueueFamilyIndices = nullptr; // Optional
            }
            this->preTransform = capabilities.currentTransform;
            this->compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            this->presentMode = chooseSwapPresentMode();
            this->clipped = VK_TRUE;
            this->oldSwapchain = VK_NULL_HANDLE;
        }
        static vk::PresentModeKHR chooseSwapPresentMode()
        {
            for (const auto& availablePresentMode : PhysicalDevice::Instance().supportedPresentModes)
            {
                if (availablePresentMode == vk::PresentModeKHR::eMailbox) // no tearing, low latency (non-submitted frames are replaced)
                {
                    return availablePresentMode;
                }
            }
            return vk::PresentModeKHR::eFifo; // no tearing, latency (every frame is submitted)
        }
        static vk::SurfaceFormatKHR chooseSwapSurfaceFormat()
        {
            for (const auto& availableFormat : PhysicalDevice::Instance().supportedSurfaceFormats)
            {
                if (availableFormat.format == vk::Format::eR8G8B8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                {
                    return availableFormat;
                }
            }
            std::unreachable();
        }

    };

    class SwapChain : public vk::raii::SwapchainKHR
    {
    public:
        SwapChain(const SwapChainCreateInfo& createInfo)
            : vk::raii::SwapchainKHR(Device::Instance(), createInfo), createInfo(createInfo)
        {
            images = this->getImages();
            // imageViews.clear();
            // for (auto& img : images)
            // {
            //     imageViews.push_back(std::make_unique<ImageView>(
            //         ImageViewCreateInfo(img, createInfo.imageFormat, vk::ImageAspectFlagBits::eColor)));
            // }
            instance = this;
        }
        uint32_t Width() { return Extent().width; }
        uint32_t Height() { return Extent().height; }
        vk::Extent2D Extent() { return createInfo.imageExtent; }
        static SwapChain& Instance()
        {
            return *instance;
        }
    public:
        // std::vector<std::unique_ptr<ImageView>> imageViews;
        std::vector<vk::Image> images;

        SwapChainCreateInfo createInfo;
        static SwapChain* instance;
    };
}