#include "pch.h"
#include "Window.h"

namespace Nork::Renderer
{
	static const char* GetDebugType(GLenum type)
	{
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:
			return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY:
			return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE:
			return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER:
			return "MARKER";
		case GL_DEBUG_TYPE_PUSH_GROUP:
			return "PUSH_GROUP";
		case GL_DEBUG_TYPE_POP_GROUP:
			return "POP_GROUP";
		case GL_DEBUG_TYPE_OTHER:
			return "OTHER";
		default:
			return "";
		}
	}
	static const char* GetDebugSource(GLenum source)
	{
		switch (source)
		{
		case GL_DEBUG_SOURCE_API:
			return "SOURCE_API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			return "WINDOW_SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			return "SHADER_COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			return "THIRD_PARTY";
		case GL_DEBUG_SOURCE_APPLICATION:
			return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER:
			return "OTHER";
		default:
			return "";
		}
	}
	static const char* GetDebugSeverity(GLenum source)
	{
		switch (source)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			return "HIGH";
		case GL_DEBUG_SEVERITY_MEDIUM:
			return "MEDIUM";
		case GL_DEBUG_SEVERITY_LOW:
			return "LOW";
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			return "NOTIFICIATION";
		default:
			return "";
		}
	}

	void APIENTRY GLDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
	{
#define nork_name_of(T) #T
		constexpr int debugSize = sizeof("GL_DEBUG__") - 1;
		constexpr char sourceOffs = debugSize + 7;
		constexpr char typeOffs = debugSize + 4;
		constexpr char severityOffs = debugSize + 8; // use these to avoid whole GL_DEBUG... stuff
		// ignore non-significant error/warning codes
		//if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		Logger::Debug("OpenGL Driver Debug Message: Severity(", GetDebugSeverity(severity), "), Type(", GetDebugType(source), "), Source(", GetDebugSource(type), ")\n\t", message);
#undef nork_name_of
	}

	void GLFWInit()
	{
		GLFWwindow* windowPtr;
		int width, height, refreshRate;
		bool isRunning;

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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