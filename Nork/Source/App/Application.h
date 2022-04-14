#pragma once

#include "Core/Engine.h"

namespace Nork
{
	class Application
	{
	public:
		static Application& Get();
		Engine engine;
	private:
		Application();
		~Application();
	};
}