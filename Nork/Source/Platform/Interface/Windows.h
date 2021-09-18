#pragma once

#include <Core/Event.h>

namespace Nork
{
	template<typename T>
	class Window
	{
	public:
		Window(int width, int height);
		~Window();
		void Refresh();
		void Close();
		void SetSize(int width, int height);
		bool IsKeyDown(int);
		bool IsRunning();
		EventManager& GetEventManager();
		T& GetData();
	};
}