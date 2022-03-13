#pragma once

#include "Components/All.h"

namespace Nork {
	template<typename T>
	concept DefaultEmplaceable = true
		&& std::_Not_same_as<T, Components::Model>;

	template<typename T>
	concept DefaultRemovable = true
		&& std::_Not_same_as<T, Components::Model>;

	class Entity
	{
	public:
		Entity(entt::entity id, entt::registry& registry)
			: id(id), registry(registry)
		{}
		template<DefaultEmplaceable T, class... A>
		inline T& AddComponent(A... args)
		{
			return registry.emplace<T>(id, args...);
		}
		Components::Model& AddModel()
		{
			return registry.emplace<Components::Model>(id, Components::Model{ .meshes = { Renderer::Mesh::Cube() } });
		}
		template<DefaultRemovable T>
		inline bool RemoveComponent()
		{
			return registry.remove<T>(id) == 1;
		}
		template<std::same_as<Components::Model> T>
		inline bool RemoveComponent()
		{
			return registry.remove<Components::Model>(id) == 1;
		}
		template<class T>
		inline T& GetComponent()
		{
			return registry.get<T>(id);
		}
		template<class T>
		inline T* TryGetComponent()
		{
			return registry.try_get<T>(id);
		}
		template<class T>
		inline const T& GetComponent() const
		{
			return registry.get<T>(id);
		}
		template<class T>
		inline const T* TryGetComponent() const
		{
			return registry.try_get<T>(id);
		}
		template<class... T>
		inline bool HasAnyComponentsOf() const
		{
			return registry.any_of<T...>(id);
		}
		template<class... T>
		inline bool HasAllComponentsOf() const
		{
			return registry.all_of<T...>(id);
		}
		entt::entity Id() const { return id; }
	private:
		entt::entity id;
		entt::registry& registry;
	};
}