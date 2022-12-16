#include "pch.h"
#include "NorkWindow.h"
#include "App/Application.h"

namespace Nork
{
	Window::Window()
		: input(nullptr)
	{
	}

	void Window::Close()
	{
		// win.Close();
	}
	void Window::SetupCallbacks()
	{
		//auto windowPtr = win.GetContext().glfwWinPtr;

		//glfwSetWindowUserPointer(windowPtr, this);

		// glfwSetWindowSizeCallback(windowPtr, [](GLFWwindow* winPtr, GLint width, GLint height)
		// 	{
		// 		Application::Get().engine.window.Underlying().OnResize(width, height);
		// 	});
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

