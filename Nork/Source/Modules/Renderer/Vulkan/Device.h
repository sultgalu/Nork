#pragma once

#include "VkSuccess.h"
#include "VulkanContext.h"

namespace Nork::Renderer::Vulkan {

    class PhysicalDevice : public vk::raii::PhysicalDevice
    {
    public:
        PhysicalDevice(const vk::raii::PhysicalDevice& dev, const vk::SurfaceKHR& surface)
            : vk::raii::PhysicalDevice(dev)
        {
            supportedSurfaceFormats = getSurfaceFormatsKHR(surface);
            supportedPresentModes = getSurfacePresentModesKHR(surface);
            memProperties = getMemoryProperties();
            physicalDeviceProperties = getProperties();
            setGraphicsQueueFamily();
            setPresentQueueFamily(surface);
            instance = this;
        }
        void setGraphicsQueueFamily()
        {
            uint32_t i = 0;
            for (auto& fam : getQueueFamilyProperties())
            {
                if (fam.queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    graphicsQueueFamily = i;
                    return;
                }
                i++;
            }
            throw std::runtime_error("found no queue family with VK_QUEUE_GRAPHICS_BIT");
        }
        void setPresentQueueFamily(const vk::SurfaceKHR& surface)
        {
            uint32_t i = 0;
            for (auto& fam : getQueueFamilyProperties())
            {
                if (this->getSurfaceSupportKHR(i, surface))
                {
                    presentQueueFamily = i;
                    return;
                }
                i++;
            }
            throw std::runtime_error("found no queue family with Present Support");
        }
        static PhysicalDevice& Instance()
        {
            return *instance;
        }
    public:
        uint32_t graphicsQueueFamily;
        uint32_t presentQueueFamily;
        vk::PhysicalDeviceProperties physicalDeviceProperties;
        vk::PhysicalDeviceMemoryProperties memProperties;
        std::vector<vk::SurfaceFormatKHR> supportedSurfaceFormats;
        std::vector<vk::PresentModeKHR> supportedPresentModes;
        static PhysicalDevice* instance;
    };

    struct DeviceCreateInfo : vk::DeviceCreateInfo
    {
        DeviceCreateInfo(uint32_t graphicsFamily, uint32_t presentFamily, const std::vector<const char*>& deviceExtensions)
        {
            const float queuePriority = 1.0f;
            vk::DeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.queueFamilyIndex = 0; // will set
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfo.queueFamilyIndex = graphicsFamily;
            queueCreateInfos.push_back(queueCreateInfo);
            if (graphicsFamily != presentFamily)
            {
                queueCreateInfo.queueFamilyIndex = presentFamily;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            deviceFeatures.samplerAnisotropy = VK_TRUE;
            deviceFeatures.multiDrawIndirect = VK_TRUE;
            // non-uniform indexing
            deviceFeatures2.shaderSampledImageArrayNonUniformIndexing = true;
            deviceFeatures2.runtimeDescriptorArray = true;
            deviceFeatures2.descriptorBindingVariableDescriptorCount = true;
            deviceFeatures2.descriptorBindingPartiallyBound = true;
            // timeline semaphores
            deviceFeatures2.timelineSemaphore = true;

            this->queueCreateInfoCount = queueCreateInfos.size();
            this->pQueueCreateInfos = queueCreateInfos.data();
            this->pEnabledFeatures = &deviceFeatures;
            this->enabledExtensionCount = deviceExtensions.size();
            this->ppEnabledExtensionNames = deviceExtensions.data();
            this->pNext = &deviceFeatures2;
        }
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        vk::PhysicalDeviceFeatures deviceFeatures;
        vk::PhysicalDeviceVulkan12Features deviceFeatures2;
    };

    class Device : public vk::raii::Device
    {
    public:
        Device(const std::vector<const char*>& deviceExtensions, const PhysicalDevice& physicalDevice)
            : vk::raii::Device(physicalDevice, DeviceCreateInfo(PhysicalDevice::Instance().graphicsQueueFamily,
                PhysicalDevice::Instance().presentQueueFamily, deviceExtensions)),
            graphicsQueue(getQueue(PhysicalDevice::Instance().graphicsQueueFamily, 0)),
            presentQueue(getQueue(PhysicalDevice::Instance().presentQueueFamily, 0))
        {
            instance = this;
        }

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
        {
            for (uint32_t i = 0; i < PhysicalDevice::Instance().memProperties.memoryTypeCount; i++)
            {
                if ((typeFilter & (1 << i)) && (PhysicalDevice::Instance().memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable memory type!");
        }
        static Device& Instance()
        {
            return *instance;
        }
    public:
        static Device* instance;

        vk::raii::Queue graphicsQueue; // cleaned up by device destroy
        vk::raii::Queue presentQueue;
    };
}