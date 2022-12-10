#pragma once

#include "VkSuccess.h"
#include "VulkanContext.h"

class Device
{
public:
    Device(const VulkanContext& ctx)
    {
        instance = this;
        physicalDevice = choosePhysicalDevice(ctx);
        createLogicalDevice();
        swapChainSupport = querySwapChainSupport(physicalDevice, ctx.surface);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    }
    VkPhysicalDevice choosePhysicalDevice(const VulkanContext& ctx)
    {
        for (const auto& device : ctx.physicalDevices())
        {
            if (isDeviceSuitable(device, ctx.surface))
            {
                return device;
            }
        }
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    ~Device()
    {
        vkDestroyDevice(device, nullptr);
    }
    void createLogicalDevice()
    {
        const float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = 0; // will set
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfo.queueFamilyIndex = this->graphicsQueueFamily;
        queueCreateInfos.push_back(queueCreateInfo);
        if (this->presentQueueFamily != this->graphicsQueueFamily)
        {
            queueCreateInfo.queueFamilyIndex = this->presentQueueFamily;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = deviceExtensions.size();
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // this is only required in older driver versions, already specified for physical device
        // if (enableValidationLayers)
        // {
        //     createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        //     createInfo.ppEnabledLayerNames = validationLayers.data();
        // }
        // else
        // {
        //     createInfo.enabledLayerCount = 0;
        // }
        vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) == VkSuccess();
        vkGetDeviceQueue(device, this->graphicsQueueFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, this->presentQueueFamily, 0, &presentQueue);
    }

    void setQueueFamilies(VkSurfaceKHR surface)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        int i = -1;
        while (++i < queueFamilyCount && !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT));
        if (i == queueFamilyCount)
            throw std::runtime_error("found no queue family with VK_QUEUE_GRAPHICS_BIT");
        this->graphicsQueueFamily = i - 1;


        i = 0;
        VkBool32 presentSupport = false;
        do
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        } while (!presentSupport && ++i < queueFamilyCount);
        if (i == queueFamilyCount)
            throw std::runtime_error("found no queue family with Present Support");
        this->presentQueueFamily = i - 1;
    }

    struct SwapChainSupportDetails
    {
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        SwapChainSupportDetails details;
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        if (!checkDeviceExtensionSupport(device))
            return false;
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
        return swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }
    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::unordered_set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        vkCreateImageView(device, &viewInfo, nullptr, &imageView) == VkSuccess();

        return imageView;
    }
    static Device& Instance()
    {
        return *instance;
    }
    VkPhysicalDeviceProperties PhysicalDeviceProperties()
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        return properties;
    }
public:
    static Device* instance;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    uint32_t graphicsQueueFamily;
    uint32_t presentQueueFamily;
    VkQueue graphicsQueue; // cleaned up by device destroy
    VkQueue presentQueue;

    SwapChainSupportDetails swapChainSupport;
    VkPhysicalDeviceMemoryProperties memProperties;

    const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
};

Device* Device::instance = nullptr;