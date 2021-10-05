#include "pch.h"
#include "Utils/Logger.h"
#include "Platform/Windows.h"
#include "Core/Event.h"
#include "Platform/Input.h"
#include "Core/Input.h"

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
	Input::Input input = Input::Input();

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

	inline EventManager& GetEventManager()
	{
		return *static_cast<EventManager*>(glfwGetWindowUserPointer(windowPtr));
	}

	void SetupEventCallbacks()
	{
		using namespace Events;

		glfwSetWindowCloseCallback(windowPtr, [](GLFWwindow* winPtr)
			{
				input.GetEventManager().RaiseEvent(WindowClose(true));
			});
		glfwSetWindowSizeCallback(windowPtr, [](GLFWwindow* winPtr, GLint width, GLint height)
			{
				input.GetEventManager().RaiseEvent(WindowResize(width, height));
			});
		glfwSetKeyCallback(windowPtr, [](GLFWwindow* winPtr, int key, int scancode, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					input.GetEventManager().RaiseEvent(KeyDown(static_cast<Key>(key)));
				}
				else if (action == GLFW_RELEASE)
				{
					input.GetEventManager().RaiseEvent(KeyUp(static_cast<Key>(key)));
				}
			});
		glfwSetMouseButtonCallback(windowPtr, [](GLFWwindow* winPtr, int button, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					input.GetEventManager().RaiseEvent(MouseDown(static_cast<MouseButton>(button)));
				}
				else if (action == GLFW_RELEASE)
				{
					input.GetEventManager().RaiseEvent(MouseUp(static_cast<MouseButton>(button)));
				}
			});
		glfwSetCursorPosCallback(windowPtr, [](GLFWwindow* winPtr, double xPos, double yPos)
			{
				input.GetEventManager().RaiseEvent(MouseMove(xPos, yPos));
			});
		glfwSetScrollCallback(windowPtr, [](GLFWwindow* winPtr, double xOff, double yOff)
			{
				input.GetEventManager().RaiseEvent(MouseScroll(yOff));
			});
		glfwSetCharCallback(windowPtr, [](GLFWwindow* winPtr, unsigned int character)
			{
				input.GetEventManager().RaiseEvent(Type(character));
			});
		glfwSetCursorEnterCallback(windowPtr, [](GLFWwindow* winPtr, int didEnter)
			{
				input.GetEventManager().RaiseEvent(didEnter == GLFW_TRUE ? CursorEnteredWindow().As<InputEvent>() : CursorLeftWindow());
			});
		glfwSetMonitorCallback([](GLFWmonitor* monitor, int didConnect)
			{
				input.GetEventManager().RaiseEvent(didConnect == GLFW_TRUE ? MonitorConnect().As<InputEvent>() : MonitorDisconnect());
			});
		glfwSetWindowFocusCallback(windowPtr, [](GLFWwindow* winPtr, int focused)
			{
				input.GetEventManager().RaiseEvent(focused == GLFW_TRUE ? WindowInFocus().As<InputEvent>() : WindowOutOfFocus());
			});

		//input = Nork::Input::Input();
		input.GetEventManager().Subscribe<WindowClose>(&OnClose);
		input.GetEventManager().Subscribe<WindowResize>(&OnResize);
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // is Depth Clearing necessary?
		glfwPollEvents();
		input.GetEventManager().PollEvents();
	}
	void _Window<GLFWwindow>::Close()
	{
		input.GetEventManager().RaiseEvent(Events::WindowClose(true));
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
	Input::Input& _Window<GLFWwindow>::GetInput()
	{
		return input;
	}
	GLFWwindow& _Window<GLFWwindow>::GetData()
	{
		return *windowPtr;
	}
}