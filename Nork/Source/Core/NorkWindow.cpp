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
	inline Event::Sender& GetEventSender()
	{
		return Application::Get().dispatcher.GetSender();
	}
	void Window::SetupCallbacks()
	{
		using namespace Event::Types;
		auto windowPtr = win.GetContext().glfwWinPtr;

		glfwSetWindowUserPointer(windowPtr, this);

		glfwSetWindowCloseCallback(windowPtr, [](GLFWwindow* winPtr)
			{
				GetEventSender().Send(WindowClose(true));
				GetWindow(winPtr).running = false;
			});
		glfwSetWindowSizeCallback(windowPtr, [](GLFWwindow* winPtr, GLint width, GLint height)
			{
				GetEventSender().Send(WindowResize(width, height));
			});
		glfwSetKeyCallback(windowPtr, [](GLFWwindow* winPtr, int key, int scancode, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					GetEventSender().Send(KeyDown(static_cast<Key>(key)));
				}
				else if (action == GLFW_RELEASE)
				{
					GetEventSender().Send(KeyUp(static_cast<Key>(key)));
				}
			});
		glfwSetMouseButtonCallback(windowPtr, [](GLFWwindow* winPtr, int button, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					GetEventSender().Send(MouseDown(static_cast<MouseButton>(button)));
				}
				else if (action == GLFW_RELEASE)
				{
					GetEventSender().Send(MouseUp(static_cast<MouseButton>(button)));
				}
			});
		glfwSetCursorPosCallback(windowPtr, [](GLFWwindow* winPtr, double xPos, double yPos)
			{
				GetEventSender().Send(MouseMove(xPos, yPos));
			});
		glfwSetScrollCallback(windowPtr, [](GLFWwindow* winPtr, double xOff, double yOff)
			{
				GetEventSender().Send(MouseScroll(yOff));
			});
		glfwSetCharCallback(windowPtr, [](GLFWwindow* winPtr, unsigned int character)
			{
				GetEventSender().Send(Type(character));
			});
		glfwSetCursorEnterCallback(windowPtr, [](GLFWwindow* winPtr, int didEnter)
			{
				if (didEnter == GLFW_TRUE) GetEventSender().Send(CursorEnteredWindow());
				else GetEventSender().Send(CursorLeftWindow());
			});
		glfwSetMonitorCallback([](GLFWmonitor* monitor, int didConnect)
			{
				if (didConnect == GLFW_TRUE) GetEventSender().Send(MonitorConnect());
				else GetEventSender().Send(MonitorDisconnect());
			});
		glfwSetWindowFocusCallback(windowPtr, [](GLFWwindow* winPtr, int focused)
			{
				if (focused == GLFW_TRUE) GetEventSender().Send(WindowInFocus());
				else GetEventSender().Send(WindowOutOfFocus());
			});

		using namespace Event::Types;
		Application::Get().dispatcher.GetReceiver().Subscribe<WindowResize>([&](const WindowResize& ev)
			{
				win.OnResize(ev.width, ev.height);
			});
	}
}

