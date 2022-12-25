#pragma once

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

}