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
		Id CreateEntity(Id hint)
		{
			return reg.create(hint);
		}
		Id CreateEntity()
		{
			return reg.create();
		}
		template<typename T>
		T& Emplace(Id id)
		{
			return reg.emplace<T>(id);
		}
		template<typename T, typename... A>
		T& Emplace(Id id, A... args)
		{
			return reg.emplace<T>(id, args...);
		}
		template<typename... T>
		auto GetComponents()
		{
			return Query<T...>(reg);
		}
	private:
		entt::registry reg;
	};
}

