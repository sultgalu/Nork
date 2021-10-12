#pragma once

namespace Nork::ECS
{
	typedef entt::entity Id;

	template<typename... T>
	struct TypeList
	{
		const static constinit size_t size = sizeof...(T);
	};
 
	template<typename... T>
	struct LazyQuery 
	{
		LazyQuery(const entt::registry& reg) : reg(reg) {}
		template<typename F>
		void ForEach(F func) const
		{
			reg.view<T...>().each(func);
		}

	private:
		const entt::registry& reg;
	};

	class Registry
	{
	public:
		inline Id CreateEntity(Id hint)
		{
			return reg.create(hint);
		}
		inline Id CreateEntity()
		{
			return reg.create();
		}
		template<typename T>
		inline T& Emplace(Id id)
		{
			return reg.emplace<T>(id);
		}
		template<typename... T>
		inline auto Remove(Id id)
		{
			return reg.remove<T...>(id);
		}
		template<typename T, typename... A>
		inline T& Emplace(Id id, A... args)
		{
			return reg.emplace<T>(id, args...);
		}	
		template<typename T, typename... A>
		inline T& EmplaceOrReplace(Id id, A... args)
		{
			return reg.emplace_or_replace<T>(id, args...);
		}
		template<typename... T>
		inline auto GetComponents() const
		{
			return LazyQuery<T...>(reg);
		}
		template<typename... T>
		inline auto HasAny(ECS::Id id) const
		{
			return reg.any_of<T...>(id);
		}
		inline const entt::registry& GetUnderlying() const
		{
			return reg;
		}
		inline entt::registry& GetUnderlyingMutable()
		{
			return reg;
		}
		inline void Clear()
		{
			reg.clear();
		}
	private:
		entt::registry reg;
	};
}

