#include "GLManager.h"

namespace Nork::Renderer {
	GLManager& GLManager::Get()
	{
		static GLManager manager;
		return manager;
	}
}
