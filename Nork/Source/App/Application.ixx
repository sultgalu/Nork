module;

// #include "Core/Engine.h"
#include "Core/Engine.h"

export module Application;

// import Engine;

export namespace Nork
{
	export class Application
	{
	public:
		static Application& Get()
		{
			static Application app;
			return app;
		}
		Engine engine;
	private:
		Application()
			: engine()
		{

		}
		~Application()
		{

		}
	};
}