#include "Window.h"

namespace Nork::Renderer::Vulkan {

#ifdef NDEBUG
	const static std::vector<const char*> validationLayers = {};
	const static bool enableValidationLayers = false;
#else
	const static std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	const static bool enableValidationLayers = true;
#endif

	void Window::Close()
	{
		glfwSetWindowShouldClose(glfwWindow, 1);
		glfwPollEvents();
		glfwTerminate();
	}
	bool Window::ShouldClose()
	{
		return glfwWindowShouldClose(glfwWindow);
	}
	void Window::SetupCallbacks()
	{
		// auto windowPtr = glfwWindow;
		// 
		// glfwSetWindowUserPointer(windowPtr, this);
		// 
		// glfwSetWindowSizeCallback(windowPtr, [](GLFWwindow* winPtr, GLint width, GLint height)
		// 	{
		// 		// Application::Get().engine.window.Underlying().OnResize(width, height);
		// 	});
	}
	void Window::Resize(int w, int h)
	{
		glfwSetWindowSize(glfwWindow, w, h);
	}
	void Window::CreateInstance()
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
		instance = std::make_unique<class Instance>(InstanceCreateInfo(VK_API_VERSION_1_3, extensions, validationLayers, debugCreateInfo));
	}
}