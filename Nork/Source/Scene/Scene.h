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
		void SaveAs(std::string path);
		void Save();
		inline void Reset()
		{
			registry = entt::registry();
		}
	public:
		entt::registry registry;
		std::shared_ptr<SceneNode> root;
		fs::path sceneUri;
	};
}