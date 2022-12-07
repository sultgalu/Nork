#pragma once
#include "VkSuccess.h"


class VulkanWindow
{
public:
    VulkanWindow(uint32_t width, uint32_t height)
        : width(width), height(height)
    {
        initWindow();
    }
    ~VulkanWindow()
    {
        glfwDestroyWindow(glfwWindow);
        glfwTerminate();
    }
    VkSurfaceKHR createSurface(VkInstance instance) const
    {
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(instance, glfwWindow, nullptr, &surface) == VkSuccess();
        return surface;
    }
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto self = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
        self->width = width;
        self->height = height;
        if (self->onFbResize)
        {
            self->onFbResize(width, height);
        }
    }
    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindow = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
        glfwSetWindowUserPointer(glfwWindow, this);
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
public:
    uint32_t width, height;

    GLFWwindow* glfwWindow;

    std::function<void(int, int)> onFbResize = nullptr;
};