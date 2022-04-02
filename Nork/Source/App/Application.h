#pragma once

#include "Core/Engine.h"

namespace Nork
{
	class Application
	{
	public:
		static Application& Get();
		Nork::Window window;
		Engine engine;
	private:
		Application();
		~Application();
	};
}