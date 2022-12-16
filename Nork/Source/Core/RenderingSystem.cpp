#include "RenderingSystem.h"
#include "Modules/Renderer/LoadUtils.h"

namespace Nork {
	RenderingSystem::RenderingSystem(entt::registry& registry)
		: registry(registry)
	{
	}
}