#pragma once

#include "ResourceManager.h"
#include "Components/All.h"

namespace Nork::Scene
{
	typedef uint64_t uuid;
	using namespace Renderer::Data;
	class Scene
	{
		template<typename T, typename... A>
		T& AddComponent(ECS::Id id, A... args)
		{
			registry.Emplace<T>(id, args...);
		}
		template<>
		Components::Model& AddComponent<Components::Model, std::string>(ECS::Id id, std::string src)
		{
			return registry.Emplace<Components::Model>(id, GetModelByResource(resMan.GetMeshes(src)));
		}
		template<>
		Components::Model& AddComponent<Components::Model>(ECS::Id id)
		{
			return registry.Emplace<Components::Model>(id, GetModelByResource(resMan.GetCube()));
		}	
	private:
		Components::Model GetModelByResource(std::vector<Renderer::Data::MeshResource> resource)
		{
			Components::Model model;
			model.meshes.reserve(resource.size());
			for (size_t i = 0; i < resource.size(); i++)
			{
				model.meshes.push_back(Renderer::Data::Mesh(resource[i]));
			}
			return model;
		}
		uuid GenUniqueId()
		{
			return std::rand();
		}
	private:
		ECS::Registry registry;
		ResourceManager resMan;
	};
}