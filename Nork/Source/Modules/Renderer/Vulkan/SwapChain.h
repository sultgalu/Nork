#pragma once
#include "Device.h"

namespace Nork::Renderer::Vulkan {
    struct SwapChainCreateInfo : vk::SwapchainCreateInfoKHR
    {
        SwapChainCreateInfo(vk::SurfaceKHR surface, const vk::SurfaceCapabilitiesKHR& capabilities,
            uint32_t imgCount, VkExtent2D extent);
        static vk::PresentModeKHR chooseSwapPresentMode();
        static vk::SurfaceFormatKHR chooseSwapSurfaceFormat();

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