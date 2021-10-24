#include "pch.h"
#include "Utils/Logger.h"
#include "Platform/Window.h"
#include "Core/Event.h"
#include "Platform/Input.h"
#include "Core/Input.h"

using namespace Nork::Input;
using namespace Nork::Input::Types;

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

	void GLFWInit()
	{
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

	void OnClose(const Event::Types::WindowClose& e)
	{
		if (!e.calledByWindow)
		{
			glfwSetWindowShouldClose(windowPtr, 1);
		}
		isRunning = false;
	}
	void OnResize(const Event::Types::WindowResize& e)
	{
		width = e.width;
		height = e.height;
	}

	/*inline Event::Registry& GetEventManager()
	{
		return *static_cast<Event::Registry*>(glfwGetWindowUserPointer(windowPtr));
	}*/

	inline Event::Sender& GetEventSender()
	{
		return static_cast<Event::Dispatcher*>(glfwGetWindowUserPointer(windowPtr))->GetSender();
	}

	void _Window<GLFWwindow>::SetupEventCallbacks()
	{
		using namespace Event::Types;

		glfwSetWindowUserPointer(windowPtr, &this->reg);

		glfwSetWindowCloseCallback(windowPtr, [](GLFWwindow* winPtr)
			{
				GetEventSender().Send(WindowClose(true));
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

		reg.GetReceiver().Subscribe<WindowClose>(&OnClose);
		reg.GetReceiver().Subscribe<WindowResize>(&OnResize);
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
	}
	void _Window<GLFWwindow>::Close()
	{
		reg.GetSender().Send(Event::Types::WindowClose(true));
	}
	void _Window<GLFWwindow>::SetSize(int w, int h)
	{
		width = w;
		height = h;
		glfwSetWindowSize(windowPtr, width, height);
	}
	glm::vec2 _Window<GLFWwindow>::Resolution()
	{
		return glm::vec2(width, height);
	}
	bool _Window<GLFWwindow>::IsRunning()
	{
		return isRunning;
	}
	GLFWwindow& _Window<GLFWwindow>::GetData()
	{
		return *windowPtr;
	}
	void Input::StateListener::QueryCurrentState()
	{
		static constinit size_t size = sizeof(keys) / sizeof(State::Key);
		for (size_t i = 0; i < size; i++)
		{
			keys[i] = glfwGetKey(windowPtr, i) == GLFW_PRESS ? KeyState::Down : KeyState::Up;
		}
		for (size_t i = 0; i < size; i++)
		{
			mouseButtons[i] = glfwGetKey(windowPtr, i) == GLFW_PRESS ? MouseButtonState::Down : MouseButtonState::Up;
		}
	}
}