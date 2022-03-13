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
		inline Components::Camera& GetMainCamera()
		{
			return mainCameraNode->GetEntity().GetComponent<Components::Camera>();
		}
	public:
		entt::registry registry;
		SceneNode root;
		std::shared_ptr<SceneNode> mainCameraNode;
	};
}