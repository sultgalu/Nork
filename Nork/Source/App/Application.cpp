#include "Application.h"

namespace Nork
{
	static EngineConfig GetEngineConfig()
	{
		return EngineConfig().SetResolution(1920, 1080);
	}

	Application& Application::Get()
	{
		static Application app;
		return app;
	}

	Application::Application():	
		window(),
		dispatcher(Dispatcher()),
		inputState(dispatcher.GetReceiver()),
		engine(Engine(GetEngineConfig()))
	{
	}
	Application::~Application()
	{
	}

}