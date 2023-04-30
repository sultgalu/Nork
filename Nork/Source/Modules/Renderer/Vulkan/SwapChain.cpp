#include "SwapChain.h"

namespace Nork::Renderer::Vulkan {
SwapChainCreateInfo::SwapChainCreateInfo(vk::SurfaceKHR surface, const vk::SurfaceCapabilitiesKHR& capabilities,
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
    // this->flags = vk::SwapchainCreateFlagBitsKHR::eMutableFormat;
    this->imageColorSpace = format.colorSpace;
    this->imageExtent = extent;
    this->imageArrayLayers = 1;
    this->imageUsage = vk::ImageUsageFlagBits::eTransferDst; //::eColorAttachment;

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
vk::PresentModeKHR SwapChainCreateInfo::chooseSwapPresentMode()
{
    for (const auto& availablePresentMode : PhysicalDevice::Instance().supportedPresentModes)
    {
        // if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
        //     return availablePresentMode;
        // }
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) { // no tearing, low latency (non-submitted frames are replaced) 
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo; // no tearing, latency (every frame is submitted)
}
vk::SurfaceFormatKHR SwapChainCreateInfo::chooseSwapSurfaceFormat()
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

}