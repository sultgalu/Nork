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
		bool IsKeyDown(int);
		void SetSize(int width, int height);
		bool IsRunning();
		EventManager& GetEventManager();
	};
}