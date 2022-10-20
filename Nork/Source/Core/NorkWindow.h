#pragma once

import Nork.Renderer;
#include "Core/InputState.h"

namespace Nork
{
	class Window
	{
	public:
		Window();
		void Close();
		void SetupCallbacks();
		bool ShouldClose() { return glfwWindowShouldClose(win.GetContext().glfwWinPtr); }
		//inline bool IsRunning() { return running; }
		inline void Resize(int w, int h) { win.Resize(w, h); }
		inline void Refresh() { win.Refresh(); input.Update(); win.PollEvents(); }
		inline Renderer::Window& Underlying() { return win; }
		inline const Nork::Input& Input() { return input; }
	private:
		Renderer::Window win;
		Nork::Input input;
		//bool running = false;
		bool refresh = true;
		int maxFps = 1000;
	};
}