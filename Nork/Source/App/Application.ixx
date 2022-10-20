export module Application;

import Nork.Core;

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