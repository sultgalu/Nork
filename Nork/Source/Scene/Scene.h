#pragma once

#include "Components/All.h"

namespace Nork
{
	template<typename T>
	concept DefaultEmplaceable = true
		&& std::_Not_same_as<T, Components::Model>;

	template<typename T>
	concept DefaultRemovable = true
		&& std::_Not_same_as<T, Components::Model>;

	typedef uint64_t uuid;
	class Scene
	{
	public:
		inline entt::entity CreateNode()
		{
			return registry.create();
		}
		inline void DeleteNode(entt::entity id)
		{
			if (registry.any_of<Components::Model>(id))
			{
				RemoveComponent<Components::Model>(id);
			}
			registry.destroy(id);
		}
		template<DefaultEmplaceable T, typename... A>
		inline T& AddComponent(entt::entity id, A... args)
		{
			return registry.emplace<T>(id, args...);
		}
		Components::Model& AddModel(entt::entity id)
		{
			return registry.emplace<Components::Model>(id, GetDefaultCube());
		}
		/*Components::Model& AddModel(entt::entity id, std::string src = "")
		{
			ownedModels[id] = src;
			return registry.emplace<Components::Model>(id, GetModelByResource(
				src._Equal("") ? resMan.GetCube() : resMan.GetMeshes(src)));
		}*/
		template<DefaultRemovable T>
		inline bool RemoveComponent(entt::entity id)
		{
			return registry.remove<T>(id) == 1;
		}
		template<std::same_as<Components::Model> T>
		inline bool RemoveComponent(entt::entity id)
		{
			return registry.remove<Components::Model>(id) == 1;
		}
		void Load(std::string path);
		void Save(std::string path);
		inline void Reset()
		{
			registry = entt::registry();
		}
		inline Components::Camera& GetMainCamera()
		{
			if (MainCameraNode == entt::null || !registry.any_of<Components::Camera>(MainCameraNode))
			{
				auto view = registry.view<Components::Camera>();
				if (view.size() > 0)
				{
					MainCameraNode = view.front();
				}
				else
				{
					MainCameraNode = CreateNode();
					AddComponent<Components::Camera>(MainCameraNode);
				}
			}
			auto& cam = registry.get<Components::Camera>(MainCameraNode);
			return cam;
		}
	private:
		static Components::Model GetDefaultCube()
		{
			static auto cube = CreateDefaultCube();
			return cube;
		}
		static Components::Model CreateDefaultCube();
		uuid GenUniqueId();

	public:
		entt::entity MainCameraNode;
		entt::registry registry;
	};
}