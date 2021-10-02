#include "pch.h"
#include "Utils/Logger.h"
#include "Platform/Interface/Windows.h"
#include "Core/Event.h"
#include "Platform/Input.h"

using namespace Nork::Input;

namespace Nork
{
	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
	{
#define nork_name_of(T) #T
		constexpr int debugSize = sizeof("GL_DEBUG__") - 1;
		constexpr char sourceOffs = debugSize + 7;
		constexpr char typeOffs = debugSize + 4;
		constexpr char severityOffs = debugSize + 8; // use these to avoid whole GL_DEBUG... stuff
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		//std::cout << "(GL_DEBUG_MSG:" << id << "): " << message << std::endl;
		Logger::Debug("OpenGL Driver Debug Message: Severity(", nork_name_of(severity), "), Source(", nork_name_of(source), "), Type(", nork_name_of(type), ")\n\t", message);
#undef nork_name_of
	}

	GLFWwindow* windowPtr;
	int width, height, refreshRate;
	bool isRunning;
	std::bitset<ToInt(Key::Max)> keys;
	EventManager eventMan;

	void GLFWInit()
	{
		std::bitset<12> set;
		set[3].flip();
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1); // DEBUG MODE
#endif
		Logger::Info("GLFW initialized");
	}

	void SetupWindow()
	{
		const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		refreshRate = vidmode->refreshRate;
		windowPtr = glfwCreateWindow(width, height, "Nork", nullptr, nullptr);

		if (!windowPtr)
		{
			Logger::Error("glfwCreateWindow returned nullptr");
			std::abort();
		}
		glfwMakeContextCurrent(windowPtr);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			Logger::Error("gladLoadGLLoader failed");
			std::abort();
		}
		glViewport(0, 0, width, height);
		//glfwSetWindowAspectRatio(windowPtr, 16, 9);

		int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		// use glDebugMessageInsert to insert debug messages
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			Logger::Info("GL Debug mode");
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}

		Logger::Info("GLFW window created, context made current");
	}

	void OnClose(const Event& ev)
	{
		auto& e = static_cast<const Events::WindowClose&>(ev);
		if (!e.calledByWindow)
		{
			glfwSetWindowShouldClose(windowPtr, 1);
		}
		isRunning = false;
	}
	void OnResize(const Event& ev)
	{
		auto& e = static_cast<const Events::WindowResize&>(ev);
		width = e.width;
		height = e.height;
	}
	void OnKeyDown(const Event& ev)
	{
		keys[ToInt(ev.As<Events::KeyUp>().key)].flip();
	}
	void OnKeyUp(const Event& ev)
	{
		keys[ToInt(ev.As<Events::KeyUp>().key)].flip();
	}

	inline EventManager& GetEventManager()
	{
		return *static_cast<EventManager*>(glfwGetWindowUserPointer(windowPtr));
	}

	void SetupEventCallbacks()
	{
		using namespace Events;

		glfwSetWindowCloseCallback(windowPtr, [](GLFWwindow* winPtr)
			{
				eventMan.RaiseEvent(WindowClose(true));
			});
		glfwSetWindowSizeCallback(windowPtr, [](GLFWwindow* winPtr, GLint width, GLint height)
			{
				eventMan.RaiseEvent(WindowResize(width, height));
			});
		glfwSetKeyCallback(windowPtr, [](GLFWwindow* winPtr, int key, int scancode, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					eventMan.RaiseEvent(KeyDown(static_cast<Key>(key)));
				}
				else if (action == GLFW_RELEASE)
				{
					eventMan.RaiseEvent(KeyUp(static_cast<Key>(key)));
				}
			});
		glfwSetMouseButtonCallback(windowPtr, [](GLFWwindow* winPtr, int button, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					eventMan.RaiseEvent(MouseDown(static_cast<MouseButton>(button)));
				}
				else if (action == GLFW_RELEASE)
				{
					eventMan.RaiseEvent(MouseUp(static_cast<MouseButton>(button)));
				}
			});
		glfwSetCursorPosCallback(windowPtr, [](GLFWwindow* winPtr, double xPos, double yPos)
			{
				eventMan.RaiseEvent(MouseMove(xPos, yPos));
			});
		glfwSetScrollCallback(windowPtr, [](GLFWwindow* winPtr, double xOff, double yOff)
			{
				eventMan.RaiseEvent(MouseScroll(yOff));
			});
		glfwSetCharCallback(windowPtr, [](GLFWwindow* winPtr, unsigned int character)
			{
				// GetEventManager().RaiseEvent(KeyTyped(0, character)); // keycode is 0 here.
			});
		eventMan.Subscribe<WindowClose>(&OnClose);
		eventMan.Subscribe<WindowResize>(&OnResize);
		eventMan.Subscribe<KeyDown>(&OnKeyDown);
		eventMan.Subscribe<KeyUp>(&OnKeyUp);
	}

	_Window<GLFWwindow>::_Window(int w, int h)
	{
		width = w;
		height = h;
		GLFWInit();
		SetupWindow();
		SetupEventCallbacks();
		isRunning = true;
	}

	_Window<GLFWwindow>::~_Window()
	{
		Logger::Info("Quitting...");
		glfwTerminate();
		Logger::Info("Window destroyed");
	}

	void _Window<GLFWwindow>::Refresh()
	{
		glfwSwapBuffers(windowPtr);
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // is Depth Clearing necessary?
	}
	void _Window<GLFWwindow>::Close()
	{
		eventMan.RaiseEvent(Events::WindowClose(true));
	}
	bool _Window<GLFWwindow>::IsKeyDown(int keyCode)
	{
		return keys[keyCode];
	}
	void _Window<GLFWwindow>::SetSize(int w, int h)
	{
		width = w;
		height = h;
		glfwSetWindowSize(windowPtr, width, height);
	}
	bool _Window<GLFWwindow>::IsRunning()
	{
		return isRunning;
	}
	EventManager& _Window<GLFWwindow>::GetEventManager()
	{
		return eventMan;
	}
	GLFWwindow& _Window<GLFWwindow>::GetData()
	{
		return *windowPtr;
	}
}