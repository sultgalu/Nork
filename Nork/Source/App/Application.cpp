#include "Application.h"

namespace Nork
{
	Application& Application::Get()
	{
		static Application app;
		return app;
	}

	Application::Application():	
		engine()
	{
	}
	Application::~Application()
	{
	}

}