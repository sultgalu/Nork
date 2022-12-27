#pragma once

#include "VkSuccess.h"
#include "VulkanContext.h"
#include "PhysicalDevice.h"

namespace Nork::Renderer::Vulkan {

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
            // synchronization2
            deviceFeatures3.synchronization2 = true;

            this->queueCreateInfoCount = queueCreateInfos.size();
            this->pQueueCreateInfos = queueCreateInfos.data();
            this->pEnabledFeatures = &deviceFeatures;
            this->enabledExtensionCount = deviceExtensions.size();
            this->ppEnabledExtensionNames = deviceExtensions.data();
            this->pNext = &deviceFeatures2;
            deviceFeatures2.pNext = &deviceFeatures3;
        }
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        vk::PhysicalDeviceFeatures deviceFeatures;
        vk::PhysicalDeviceVulkan12Features deviceFeatures2;
        vk::PhysicalDeviceVulkan13Features deviceFeatures3;
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