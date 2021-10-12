#pragma once

#include "ResourceManager.h"
#include "Components/All.h"

namespace Nork::Scene
{
	template<typename T>
	concept DefaultEmplaceable = true
		&& std::_Not_same_as<T, Components::Model>;

	template<typename T>
	concept DefaultRemovable = true;

	typedef uint64_t uuid;
	using namespace Renderer::Data;
	class Scene
	{
	public:
		inline ECS::Id CreateNode()
		{
			return registry.CreateEntity();
		}
		template<DefaultEmplaceable T, typename... A>
		inline T& AddComponent(ECS::Id id, A... args)
		{
			return registry.Emplace<T>(id, args...);
		}
		Components::Model& AddModel(ECS::Id id, std::string src)
		{
			ownedModels[id] = src;
			return registry.Emplace<Components::Model>(id, GetModelByResource(resMan.GetMeshes(src)));
		}
		Components::Model& AddModel(ECS::Id id)
		{
			ownedModels[id] = "";
			return registry.Emplace<Components::Model>(id, GetModelByResource(resMan.GetCube()));
		}
		template<DefaultRemovable T>
		inline bool RemoveComponent(ECS::Id id)
		{
			return registry.Remove<T>(id) == 1;
		}
		void Load(std::string path);
		void Save(std::string path);
	private:
		Components::Model GetModelByResource(std::vector<Renderer::Data::MeshResource> resource);
		uuid GenUniqueId();
	public:
		ECS::Registry registry;
		ResourceManager resMan;

		std::unordered_map<ECS::Id, std::string> ownedModels;
	};

	template<>
	inline bool Scene::RemoveComponent<Components::Model>(ECS::Id id)
	{
		if (ownedModels.contains(id))
		{ 
			if (ownedModels[id]._Equal(""))
			{
				ownedModels.erase(id);
				return registry.Remove<Components::Model>(id) == 1;
			}

			resMan.DroppedMeshResource(ownedModels[id]);
			ownedModels.erase(id);
			return registry.Remove<Components::Model>(id) == 1;
		}
		MetaLogger().Error(static_cast<size_t>(id), " is not owning any models.");
		return false;
	}
}