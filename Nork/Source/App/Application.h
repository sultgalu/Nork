#pragma once

#include "Core/Engine.h"

namespace Nork
{
	class Application
	{
	public:
		static Application& Get();
		Nork::Window window;
		Dispatcher dispatcher;
		InputState inputState;
		Engine engine;
		ResourceManager resourceManager;
	private:
		Application();
		~Application();
	};
}