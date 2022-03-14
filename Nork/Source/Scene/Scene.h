#pragma once

#include "SceneNode.h"

namespace Nork
{
	typedef uint64_t uuid;
	class Scene
	{
	public:
		Scene();
		std::shared_ptr<SceneNode> CreateNode();
		std::shared_ptr<SceneNode> CreateNode(SceneNode& parent);
		void DeleteNode(SceneNode& node);
		void Load(std::string path);
		void Save(std::string path);
		inline void Reset()
		{
			registry = entt::registry();
		}
		Components::Camera& GetMainCamera();
	public:
		entt::registry registry;
		std::shared_ptr<SceneNode> root;
		// std::weak_ptr<SceneNode> mainCameraNode;
	};
}