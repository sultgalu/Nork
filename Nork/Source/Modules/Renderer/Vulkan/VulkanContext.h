#pragma once

namespace Nork::Renderer::Vulkan {
    struct InstanceCreateInfo : vk::InstanceCreateInfo
    {
        InstanceCreateInfo(uint32_t vulkanVersion, const std::vector<const char*>& extensions,
            const std::vector<const char*>& validationLayers, const VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo)
        {
            appInfo = std::make_unique<vk::ApplicationInfo>();
            appInfo->pApplicationName = "Hello Triangle";
            appInfo->applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo->pEngineName = "No Engine";
            appInfo->engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo->apiVersion = vulkanVersion;

            pApplicationInfo = appInfo.get();

            enabledExtensionCount = extensions.size();
            ppEnabledExtensionNames = extensions.data();

            if (validationLayers.size() > 0)
            {
                enabledLayerCount = validationLayers.size();
                ppEnabledLayerNames = validationLayers.data();

                pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else
            {
                enabledLayerCount = 0;
                pNext = nullptr;
            }
        }
        std::unique_ptr<vk::ApplicationInfo> appInfo;
        std::shared_ptr<vk::raii::Context> ctx = std::make_shared<vk::raii::Context>();
    };

    class Instance : public vk::raii::Instance
    {
    public:
        Instance(const InstanceCreateInfo& createInfo)
            : vk::raii::Instance(*createInfo.ctx, createInfo), ctx(createInfo.ctx)
        {
            if (createInfo.enabledLayerCount > 0 && createInfo.pNext != nullptr)
            {
                debugMessenger = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(
                    createDebugUtilsMessengerEXT(*(VkDebugUtilsMessengerCreateInfoEXT*)createInfo.pNext));
            }
            staticInstance = this;
        }
        static Instance& StaticInstance()
        {
            return *staticInstance;
        }
    public:
        std::shared_ptr<vk::raii::Context> ctx;
        std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugMessenger;
        static Instance* staticInstance;
    };

}