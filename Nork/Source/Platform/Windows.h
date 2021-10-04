#pragma once

#include "Core/Input.h"

namespace Nork
{
	template<typename T>
	class _Window
	{
	public:
		_Window(int width, int height);
		~_Window();
		void Refresh();
		void Close();
		void SetSize(int width, int height);
		bool IsRunning();
		Input::Input& GetInput();
		T& GetData();
	};

#ifdef _WIN64
	typedef _Window<GLFWwindow> Window;
#else
	typedef _Window<int> Window;
#endif
}