#pragma once

#include <Core/Event.h>

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
		bool IsKeyDown(int);
		bool IsRunning();
		EventManager& GetEventManager();
		T& GetData();
	};

#ifdef _WIN64
	typedef _Window<GLFWwindow> Window;
#else
	typedef _Window<int> Window;
#endif
}