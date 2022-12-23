#pragma once
#include "VkSuccess.h"
#include "Core/InputState.h"
#include "SwapChain.h"

namespace Nork::Renderer::Vulkan {

    class Window
    {
    public:
        Window() = delete; 
        Window(uint32_t width, uint32_t height)
            : width(width), height(height)
        {
            InitGlfw();
            CreateInstance();
            CreateSurface();
            physicalDevice = std::make_unique<PhysicalDevice>(choosePhysicalDevice(Instance::StaticInstance()), **surface);
            device = std::make_unique<Device>(deviceExtensions, *physicalDevice);
            auto capabilities = PhysicalDevice::Instance().getSurfaceCapabilitiesKHR(**surface);
            swapchain = std::make_unique<SwapChain>(SwapChainCreateInfo(**surface, capabilities, 3, chooseSwapExtent(capabilities)));
            staticInstance = this;
        }
        ~Window()
        {
            glfwDestroyWindow(glfwWindow);
            glfwTerminate();
        }
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
        {
            auto& win = Window::Instance();
            win.width = width;
            win.height = height;
            if (win.onFbResize)
            {
                win.onFbResize(width, height);
            }
            win.recreateSwapChain();
        }
        void recreateSwapChain()
        {
            int width = 0, height = 0;
            while (width == 0 || height == 0)
            {
                glfwGetFramebufferSize(glfwWindow, &width, &height);
                glfwWaitEvents();
            }
            vkDeviceWaitIdle(*Device::Instance());
            swapchain = nullptr;
            auto capabilities = PhysicalDevice::Instance().getSurfaceCapabilitiesKHR(**surface);
            swapchain = std::make_unique<SwapChain>(SwapChainCreateInfo(**surface, capabilities, 3, chooseSwapExtent(capabilities)));
        }
        void InitGlfw()
        {
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindow = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
            // glfwSetWindowUserPointer(glfwWindow, this);
            glfwSetFramebufferSizeCallback(glfwWindow, framebufferResizeCallback);

            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

            std::cout << extensionCount << " extensions supported\n";
            std::cout << "available extensions:\n";

            for (const auto& extension : extensions)
            {
                std::cout << '\t' << extension.extensionName << '\n';
            }
        }
        void CreateInstance()
        {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
            if (enableValidationLayers)
            {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;
            debugCreateInfo.pUserData = nullptr; // Optional

            if (!checkValidationLayerSupport(validationLayers))
                throw std::runtime_error("validation layers requested, but not available!");
            instance = std::make_unique<class Instance>(InstanceCreateInfo(VK_API_VERSION_1_2, extensions, validationLayers, debugCreateInfo));
        }
        static bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
        {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : validationLayers)
            {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers)
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0)
                    {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound)
                {
                    return false;
                }
            }

            return true;
        }
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
        {
            if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                std::string type;
                switch (messageType)
                {
                case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                    type = "GENERAL";
                case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                    type = "VALIDATION";
                case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                    type = "PERFORMANCE";
                default:
                    break;
                }
                // bug in validation (UNASSIGNED-input-attachment-descriptor-not-in-subpass) check if resolved at:
                // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4628
                if (pCallbackData->messageIdNumber == -392708513)
                    return false;
                // Message is important enough to show
                Logger::Error("VULKAN validation layer: ", type, ": ", pCallbackData->pMessage, "\n");
            }

            return false;
        }
        void CreateSurface()
        {
            VkSurfaceKHR surfaceHandle;
            glfwCreateWindowSurface(**instance, glfwWindow, nullptr, &surfaceHandle) == VkSuccess();
            surface = std::make_unique<vk::raii::SurfaceKHR>(*instance, surfaceHandle);
        }
        vk::raii::PhysicalDevice choosePhysicalDevice(const Instance& inst)
        {
            for (const auto& device : inst.enumeratePhysicalDevices())
            {
                if (isDeviceSuitable(device))
                {
                    return device;
                }
            }
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        bool isDeviceSuitable(const vk::raii::PhysicalDevice& device)
        {
            std::unordered_set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
            for (const auto& extension : device.enumerateDeviceExtensionProperties())
            {
                requiredExtensions.erase(extension.extensionName);
            }
            if (!requiredExtensions.empty())
                return false;

            return !device.getSurfaceFormatsKHR(**surface).empty() && !device.getSurfacePresentModesKHR(**surface).empty()
                && device.getFeatures().samplerAnisotropy;
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
                glfwGetFramebufferSize(glfwWindow, &width, &height);

                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                return actualExtent;
            }
        }
        static Window& Instance()
        {
            return *staticInstance;
        }

        void Close();
        void SetupCallbacks();
        bool ShouldClose();
        void Resize(int w, int h);
    public:
        uint32_t width, height;
        GLFWwindow* glfwWindow;
        std::unique_ptr<class Instance> instance;
        std::unique_ptr<vk::raii::SurfaceKHR> surface;
        std::unique_ptr<PhysicalDevice> physicalDevice;
        std::unique_ptr<Device> device;
        std::unique_ptr<SwapChain> swapchain;

        std::function<void(int, int)> onFbResize = nullptr;
        static Window* staticInstance;

        inline static const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
        };
#ifdef NDEBUG
        const std::vector<const char*> validationLayers = {};
        const static bool enableValidationLayers = false;
#else
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
        const static bool enableValidationLayers = true;
#endif
    };
}
