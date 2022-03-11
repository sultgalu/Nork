#pragma once

#include "Modules/Renderer/Window/Window.h"
#include "Core/Event.h"

namespace Nork
{
	class Window
	{
	public:
		Window();
		void Close();
		void SetupCallbacks();
		inline bool IsRunning() { return running; }
		inline void Resize(int w, int h) { win.Resize(w, h); }
		inline void Refresh() { win.Refresh(); win.PollEvents(); }
		inline Renderer::Window& Underlying() { return win; }
	private:
		Renderer::Window win;
		bool running = false;
		bool refresh = true;
		int maxFps = 1000;
	};
}