#include "pch.h"
#include "NorkWindow.h"
#include "App/Application.h"

namespace Nork
{
	Window::Window()
		: win(Renderer::Window(Renderer::WindowSetup{
			.width = 1280,
			.height = 720,
			.label = "Nork"
			}))
	{
		running = true;
	}

	inline static Window& GetWindow(GLFWwindow* ptr)
	{
		return *static_cast<Window*>(glfwGetWindowUserPointer(ptr));
	}
	void Window::Close()
	{
		win.Close();
	}
	inline Sender& GetEventSender()
	{
		return Application::Get().dispatcher.GetSender();
	}
	void Window::SetupCallbacks()
	{
		auto windowPtr = win.GetContext().glfwWinPtr;

		glfwSetWindowUserPointer(windowPtr, this);

		glfwSetWindowCloseCallback(windowPtr, [](GLFWwindow* winPtr)
			{
				GetEventSender().Send(WindowCloseEvent(true));
				GetWindow(winPtr).running = false;
			});
		glfwSetWindowSizeCallback(windowPtr, [](GLFWwindow* winPtr, GLint width, GLint height)
			{
				GetEventSender().Send(WindowResizeEvent(width, height));
			});
		glfwSetKeyCallback(windowPtr, [](GLFWwindow* winPtr, int key, int scancode, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					GetEventSender().Send(KeyDownEvent(static_cast<Key>(key)));
				}
				else if (action == GLFW_RELEASE)
				{
					GetEventSender().Send(KeyUpEvent(static_cast<Key>(key)));
				}
			});
		glfwSetMouseButtonCallback(windowPtr, [](GLFWwindow* winPtr, int button, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					GetEventSender().Send(MouseDownEvent(static_cast<MouseButton>(button)));
				}
				else if (action == GLFW_RELEASE)
				{
					GetEventSender().Send(MouseUpEvent(static_cast<MouseButton>(button)));
				}
			});
		glfwSetCursorPosCallback(windowPtr, [](GLFWwindow* winPtr, double xPos, double yPos)
			{
				GetEventSender().Send(MouseMoveEvent(xPos, yPos));
			});
		glfwSetScrollCallback(windowPtr, [](GLFWwindow* winPtr, double xOff, double yOff)
			{
				GetEventSender().Send(MouseScrollEvent(yOff));
			});
		glfwSetCharCallback(windowPtr, [](GLFWwindow* winPtr, unsigned int character)
			{
				GetEventSender().Send(TypeEvent(character));
			});
		glfwSetCursorEnterCallback(windowPtr, [](GLFWwindow* winPtr, int didEnter)
			{
				if (didEnter == GLFW_TRUE) GetEventSender().Send(CursorEnteredWindowEvent());
				else GetEventSender().Send(CursorLeftWindowEvent());
			});
		glfwSetMonitorCallback([](GLFWmonitor* monitor, int didConnect)
			{
				if (didConnect == GLFW_TRUE) GetEventSender().Send(MonitorConnectEvent());
				else GetEventSender().Send(MonitorDisconnectEvent());
			});
		glfwSetWindowFocusCallback(windowPtr, [](GLFWwindow* winPtr, int focused)
			{
				if (focused == GLFW_TRUE) GetEventSender().Send(WindowInFocusEvent());
				else GetEventSender().Send(WindowOutOfFocusEvent());
			});

		Application::Get().dispatcher.GetReceiver().Subscribe<WindowResizeEvent>([&](const WindowResizeEvent& ev)
			{
				win.OnResize(ev.width, ev.height);
			});
	}
}

