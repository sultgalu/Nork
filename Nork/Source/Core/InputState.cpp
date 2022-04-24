#include "pch.h"
#include "InputState.h"

namespace Nork {
	static Nork::Input& Self(GLFWwindow* windowPtr)
	{
		return *(Input*)glfwGetWindowUserPointer(windowPtr);
	}
	Input::Input(GLFWwindow* windowPtr)
	{
		glfwSetWindowUserPointer(windowPtr, this);

		glfwSetKeyCallback(windowPtr, [](GLFWwindow* winPtr, int key, int scancode, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					Self(winPtr).keyChanged.set(key);
					Self(winPtr).keys.set(key);
				}
				else if (action == GLFW_RELEASE)
				{
					Self(winPtr).keyChanged.set(key);
					Self(winPtr).keys.reset(key);
					Self(winPtr).keysRepeated.reset(key);
				}
				else if (action == GLFW_REPEAT) [[unlikely]]
				{
					Self(winPtr).keysRepeated.set(key);
				}
			});
		glfwSetMouseButtonCallback(windowPtr, [](GLFWwindow* winPtr, int button, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					Self(winPtr).buttonChanged.set(button);
					Self(winPtr).buttons.set(button);
				}
				else if (action == GLFW_RELEASE)
				{
					Self(winPtr).buttonChanged.set(button);
					Self(winPtr).buttons.reset(button);
				}
			});
		glfwSetCursorPosCallback(windowPtr, [](GLFWwindow* winPtr, double xPos, double yPos)
			{
				Self(winPtr).cursorX = xPos;
				Self(winPtr).cursorY = yPos;
			});
		glfwSetScrollCallback(windowPtr, [](GLFWwindow* winPtr, double xOff, double yOff)
			{
				Self(winPtr).scrollOffset = yOff;
			});
		glfwSetCharCallback(windowPtr, [](GLFWwindow* winPtr, unsigned int character)
			{
				Self(winPtr).typedChars.push_back(character);
			});
		glfwSetWindowCloseCallback(windowPtr, [](GLFWwindow* winPtr)
			{
				Self(winPtr).windowShouldClose = true;
			});
		glfwSetCursorEnterCallback(windowPtr, [](GLFWwindow* winPtr, int didEnter)
			{
				if (didEnter)
					Self(winPtr).cursorEntered = true;
				else
					Self(winPtr).cursorLeft = true;
			});
	}
	void Input::Update()
	{
		prevCursorX = cursorX;
		prevCursorY = cursorY;
		scrollOffset = 0;
		cursorEntered = false;
		cursorLeft = false;

		typedChars.clear();
		keyChanged.reset();
		buttonChanged.reset();
	}
}


