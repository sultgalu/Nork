#pragma once

#include "Scene/SceneNode.h"

namespace Nork {
	class JsonSerializer
	{
	public:
		JsonSerializer(entt::registry& reg) : registry(reg) {}
		std::string Serialize(SceneNode& node);
		std::shared_ptr<SceneNode> Deserialize(const std::string& json);
	private:
		entt::registry& registry;
	};
}