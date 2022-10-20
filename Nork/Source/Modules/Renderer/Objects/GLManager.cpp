module Nork.Renderer;

namespace Nork::Renderer {
	GLManager& GLManager::Get()
	{
		static GLManager manager;
		return manager;
	}
}
