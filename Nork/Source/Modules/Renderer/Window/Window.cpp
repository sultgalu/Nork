#include "pch.h"
#include "Window.h"

namespace Nork::Renderer
{
	void APIENTRY GLDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
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

	void GLFWInit()
	{
		GLFWwindow* windowPtr;
		int width, height, refreshRate;
		bool isRunning;

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1); // DEBUG MODE
#endif
		Logger::Info("GLFW initialized");
	}

	Window::Window(WindowSetup setup)
	{
		context.ParseSetup(setup);
		
		GLFWInit();

		const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		context.refreshRate = vidmode->refreshRate;
		context.glfwWinPtr = glfwCreateWindow(setup.width, setup.height, setup.label.c_str(), nullptr, nullptr);

		if (!context.glfwWinPtr)
		{
			Logger::Error("glfwCreateWindow returned nullptr");
			std::abort();
		}
		glfwMakeContextCurrent(context.glfwWinPtr);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			Logger::Error("gladLoadGLLoader failed");
			std::abort();
		}
		glViewport(0, 0, context.width, context.height);
		//glfwSetWindowAspectRatio(windowPtr, 16, 9);

		int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		// use glDebugMessageInsert to insert debug messages
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			Logger::Info("GL Debug mode");
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(GLDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}

		Logger::Info("GLFW window created, context made current");
	}
	void Window::Close()
	{
		glfwSetWindowShouldClose(context.glfwWinPtr, 1);
		glfwPollEvents();
		glfwTerminate();
	}
	void Window::Resize(int w, int h)
	{
		glfwSetWindowSize(context.glfwWinPtr, w, h);
	}
	void Window::Refresh()
	{
		glfwSwapBuffers(context.glfwWinPtr);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	void Window::OnResize(int newX, int newY)
	{
		context.width = newX;
		context.height = newY;
	}
}