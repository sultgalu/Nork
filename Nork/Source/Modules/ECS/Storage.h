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
	struct Query 
	{
		Query(entt::registry& reg) : reg(reg) {}
		template<typename F>
		void ForEach(F func)
		{
			reg.view<T...>().each(func);
		}

	private:
		entt::registry& reg;
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
		template<typename T, typename... A>
		inline T& Emplace(Id id, A... args)
		{
			return reg.emplace<T>(id, args...);
		}
		template<typename... T>
		inline auto GetComponents()
		{
			return Query<T...>(reg);
		}
		inline entt::registry& GetUnderlying()
		{
			return reg;
		}
	private:
		entt::registry reg;
	};
}

