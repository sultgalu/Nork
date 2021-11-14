#pragma once

#include "ResourceManager.h"
#include "Components/All.h"

namespace Nork::Scene
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
		inline ECS::Id CreateNode()
		{
			return registry.CreateEntity();
		}
		inline void DeleteNode(ECS::Id id)
		{
			if (registry.HasAny<Components::Model>(id))
			{
				RemoveComponent<Components::Model>(id);
			}
			registry.DeleteEntity(id);
		}
		template<DefaultEmplaceable T, typename... A>
		inline T& AddComponent(ECS::Id id, A... args)
		{
			return registry.Emplace<T>(id, args...);
		}
		Components::Model& AddModel(ECS::Id id, std::string src = "")
		{
			ownedModels[id] = src;
			return registry.Emplace<Components::Model>(id, GetModelByResource(
				src._Equal("") ? resMan.GetCube() : resMan.GetMeshes(src)));
		}
		template<DefaultRemovable T>
		inline bool RemoveComponent(ECS::Id id)
		{
			return registry.Remove<T>(id) == 1;
		}
		template<std::same_as<Components::Model> T>
		inline bool RemoveComponent(ECS::Id id)
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
		void Load(std::string path);
		void Save(std::string path);
		inline void Reset()
		{
			FreeResources();
			registry.Wipe();
		}
		inline Components::Camera& GetMainCamera()
		{
			if (MainCameraNode == ECS::invalidId)
			{
				auto view = registry.GetUnderlyingMutable().view<Components::Camera>();
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
			auto& cam = registry.GetComponent<Components::Camera>(MainCameraNode);
			return cam;
		}
	private:
		Components::Model GetModelByResource(std::vector<Renderer::Data::MeshResource> resource);
		void FreeResources()
		{
			auto view = registry.GetUnderlyingMutable().view<Components::Model>();
			for (auto& res : ownedModels)
			{
				if (res.second._Equal(""))
					return;
				if (!view.contains(res.first))
					Logger::Error((size_t)res.first, "(id) was holding on for a model resource but did not have a model component.");
				resMan.DroppedMeshResource(res.second);
			}
			ownedModels.clear();
		}
		uuid GenUniqueId();
	public:
		ECS::Id MainCameraNode;
		ECS::Registry registry;
		ResourceManager resMan;

		std::unordered_map<ECS::Id, std::string> ownedModels;
	};
}