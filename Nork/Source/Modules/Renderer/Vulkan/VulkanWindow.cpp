#include "Window.h"

namespace Nork::Renderer::Vulkan {
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
}