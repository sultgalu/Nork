#include "Device.h"

namespace Nork::Renderer::Vulkan {
DeviceCreateInfo::DeviceCreateInfo(uint32_t graphicsFamily, uint32_t presentFamily, const std::vector<const char*>& deviceExtensions)
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

    deviceFeatures.geometryShader = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.multiDrawIndirect = VK_TRUE;
    deviceFeatures.drawIndirectFirstInstance = VK_TRUE;
    // non-uniform indexing
    deviceFeatures2.shaderSampledImageArrayNonUniformIndexing = true;
    deviceFeatures2.runtimeDescriptorArray = true;
    deviceFeatures2.descriptorBindingVariableDescriptorCount = true;
    deviceFeatures2.descriptorBindingPartiallyBound = true;
    // timeline semaphores
    deviceFeatures2.timelineSemaphore = true;
    // synchronization2
    deviceFeatures3.synchronization2 = true;
    // mesh shader
    deviceFeaturesMeshShader.meshShader = true;

    this->queueCreateInfoCount = queueCreateInfos.size();
    this->pQueueCreateInfos = queueCreateInfos.data();
    this->pEnabledFeatures = &deviceFeatures;
    this->enabledExtensionCount = deviceExtensions.size();
    this->ppEnabledExtensionNames = deviceExtensions.data();
    this->pNext = &deviceFeatures2;
    deviceFeatures2.pNext = &deviceFeatures3;
    // deviceFeatures3.pNext = &deviceFeaturesMeshShader;
}

}