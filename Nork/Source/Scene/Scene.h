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
		std::shared_ptr<SceneNode> GetNodeById(entt::entity);

		void Create(const fs::path&);
		void Save();
		void SaveAs(const fs::path& path);
		void Load(const fs::path& path);

		void Serialize(std::ostream& os);
		void Deserialize(const std::istream& is);
		inline void Reset()
		{
			registry = entt::registry();
		}
	private:
		static std::shared_ptr<SceneNode> CreateRoot(entt::registry&);
	public:
		entt::registry registry;
		std::shared_ptr<SceneNode> root;
		fs::path sceneUri;
	};
}