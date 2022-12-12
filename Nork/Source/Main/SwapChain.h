#pragma once
#include "VulkanContext.h"
#include "Device.h"
#include "RenderPass.h"
#include "Image.h"
#include "Framebuffer.h"

class SwapChain
{
public:
    SwapChain(VulkanContext& ctx, VulkanWindow& window, Device& device, std::shared_ptr<RenderPass> renderPass)
        : ctx(ctx), window(window), device(device), renderPass(renderPass)
    {
        createSwapChain();
        // createImageViews();
        // createFramebuffers(*renderPass);
    }
    ~SwapChain()
    {
        cleanupSwapChain();
    }

    void createSwapChain()
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, ctx.surface, &capabilities) == VkSuccess();
        auto& swapChainSupport = device.swapChainSupport;

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(capabilities);
        uint32_t imageCount = 3; // triple buffering //capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        {
            imageCount = capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = ctx.surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT use VK_IMAGE_USAGE_TRANSFER_DST_BIT if it is not a destination (we need this)

        if (device.graphicsQueueFamily != device.presentQueueFamily)
        {
            uint32_t queueFamilyIndices[] = { device.graphicsQueueFamily, device.presentQueueFamily };
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        vkCreateSwapchainKHR(device.device, &createInfo, nullptr, &swapChain) == VkSuccess();
        
        vkGetSwapchainImagesKHR(device.device, swapChain, &imageCount, nullptr);
        std::vector<VkImage> images;
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(device.device, swapChain, &imageCount, images.data());
        swapChainImages.reserve(imageCount);
        swapChainImages.clear();
        for (auto handle : images)
            swapChainImages.push_back(std::make_shared<SwapChainImage>(handle, createInfo));
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window.glfwWindow, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) // no tearing, low latency (non-submitted frames are replaced)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR; // no tearing, latency (every frame is submitted)
    }
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }
        std::unreachable();
    }

    void cleanupSwapChain()
    {
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            vkDestroyImageView(device.device, swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(device.device, swapChain, nullptr);
    }

    void recreateSwapChain()
    {
        int width = 0, height = 0;
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window.glfwWindow, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device.device);

        cleanupSwapChain();

        createSwapChain();
        // createImageViews();
        // createFramebuffers(*renderPass);
    }
    /*
    void createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());
        for (uint32_t i = 0; i < swapChainImages.size(); i++)
        {
            swapChainImageViews[i] = device.createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
        depthImage = std::make_shared<Image>(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);

    }
    void createFramebuffers(const RenderPass& renderPass)
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            std::vector<VkImageView> attachments = {
                swapChainImageViews[i], depthImage->imageViewHandle
            };
            swapChainFramebuffers[i] = std::make_shared<Framebuffer>(swapChainExtent.width, swapChainExtent.height,
                renderPass, attachments);
        }
    }*/
public:
    VkSwapchainKHR swapChain;
    std::vector<std::shared_ptr<SwapChainImage>> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::shared_ptr<RenderPass> renderPass;
    std::vector<VkImageView> swapChainImageViews;
    // std::vector<std::shared_ptr<Framebuffer>> swapChainFramebuffers;

    Device& device;
    VulkanWindow& window;
    VulkanContext& ctx;
};