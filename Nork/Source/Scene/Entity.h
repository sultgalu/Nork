#pragma once

#include "Components/All.h"

namespace Nork {
	class Entity
	{
	public:
		Entity(entt::entity id, entt::registry& registry)
			: id(id), registry(registry)
		{}
		template<class T, class... A>
		inline T& AddComponent(A... args)
		{
			return registry.emplace<T>(id, args...);
		}
		template<class T>
		inline bool RemoveComponent()
		{
			return registry.remove<T>(id) == 1;
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
		inline const T* TryGetComponent() const
		{
			return registry.try_get<T>(id);
		}
		template<class T>
		inline const T& GetComponent() const
		{
			return registry.get<T>(id);
		}
		template<class... T>
		inline bool HasAnyComponentsOf() const
		{
			return registry.any_of<T...>(id);
		}
		template<class T>
		inline bool HasComponent() const
		{
			return registry.any_of<T>(id);
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