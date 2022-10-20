#include "pch.h"
#include "NorkWindow.h"
import Application;

namespace Nork
{
	Window::Window()
		: win(Renderer::Window(Renderer::WindowSetup{
			.width = 1920,
			.height = 1080,
			.label = "Nork"
			})),
		input(win.GetContext().glfwWinPtr)
	{}

	void Window::Close()
	{
		win.Close();
	}
	void Window::SetupCallbacks()
	{
		auto windowPtr = win.GetContext().glfwWinPtr;

		//glfwSetWindowUserPointer(windowPtr, this);

		glfwSetWindowSizeCallback(windowPtr, [](GLFWwindow* winPtr, GLint width, GLint height)
			{
				Application::Get().engine.window.Underlying().OnResize(width, height);
			});
		// glfwSetMonitorCallback([](GLFWmonitor* monitor, int didConnect)
		// 	{
		// 		if (didConnect == GLFW_TRUE) GetEventSender().Send(MonitorConnectEvent());
		// 		else GetEventSender().Send(MonitorDisconnectEvent());
		// 	});
		// glfwSetWindowFocusCallback(windowPtr, [](GLFWwindow* winPtr, int focused)
		// 	{
		// 		if (focused == GLFW_TRUE) GetEventSender().Send(WindowInFocusEvent());
		// 		else GetEventSender().Send(WindowOutOfFocusEvent());
		// 	});
	}
}

