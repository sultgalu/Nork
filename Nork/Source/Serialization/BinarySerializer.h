#pragma once
#include "Components/All.h"
#include "Modules/ECS/Storage.h"
#include "Scene/Scene.h"

namespace Nork::Serialization
{
	template<typename T>
	concept TriviallyCopyable = std::is_trivially_copyable<T>::value;
	template<typename T>
	concept CustomCopyable = !std::is_trivially_copyable<T>::value;

	struct PoolInfo
	{
		size_t count, typeSize;

		const entt::entity* ids;
		const void* components;

		inline size_t Size()
		{
			return sizeof(size_t) + count * (sizeof(entt::entity) + typeSize);
		}

		template<typename T>
		static PoolInfo Of(entt::registry& reg)
		{
			auto view = reg.view<T>();

			PoolInfo pool;
			pool.typeSize = sizeof(T);
			pool.count = view.size();
			if (pool.count == 0)
			{
				pool.ids = nullptr;
				pool.components = nullptr;
			}
			else
			{
				pool.ids = view.data();
				pool.components = reinterpret_cast<void*>(*view.raw());
			}
			return pool;
		}

		template<typename T>
		static PoolInfo Of(const char* src)
		{
			PoolInfo pool;
			pool.typeSize = sizeof(T);
			pool.count = *(reinterpret_cast<const size_t*>(src));
			if (pool.count == 0)
			{
				pool.ids = nullptr;
				pool.components = nullptr;
			}
			else
			{
				pool.ids = reinterpret_cast<const entt::entity*>(src + sizeof(size_t));
				pool.components = reinterpret_cast<const void*>(src + sizeof(size_t) + sizeof(entt::entity) * pool.count);
			}
			return pool;
		}

		void Serialize(char* dest)
		{
			*((size_t*)dest) = count; // list count
			if (count == 0)
				return;
			dest += sizeof(size_t);

			std::memcpy(dest, ids, sizeof(entt::entity) * count); // list of ids
			dest += sizeof(entt::entity) * count;

			std::memcpy(dest, components, typeSize * count); // list of components
		}

		template<typename T>
		void Deserialize(entt::registry& reg)
		{
			for (size_t i = 0; i < count; i++)
			{
				reg.emplace<T>(ids[i], reinterpret_cast<const T*>(components)[i]);
			}
		}

		PoolInfo() = default;
	};

	template<TriviallyCopyable... Types>
	class MemcpySerializer
	{
	public:
		using Pools = std::array<PoolInfo, sizeof...(Types)>;

		static std::vector<char> Serialize(entt::registry& reg)
		{
			Pools pools;
			AddPool<0, Types...>(reg, pools);

			std::vector<char> result(CalcSize(pools), '\0');
			char* buf = result.data();

			for (size_t i = 0; i < pools.size(); i++)
			{
				pools[i].Serialize(buf);
				buf += pools[i].Size();
			}

			return result;
		}

		static size_t Deserialize(entt::registry& reg, const char* src)
		{
			Pools pools;
			AddPool<0, Types...>(src, pools);

			Deserialize<0, Types...>(reg, pools);
			return CalcSize(pools);
		}

	private:
		template<size_t i, typename T, typename... Rest>
		static void Deserialize(entt::registry& reg, Pools& pools)
		{
			pools[i].Deserialize<T>(reg);
			if constexpr (sizeof...(Rest) > 1)
			{
				Deserialize<i + 1, Rest...>(reg, pools);
			}
			else
			{
				pools[i + 1].Deserialize<Rest...>(reg);
			}
		}
		template<size_t i, typename T, typename... Rest>
		static void AddPool(entt::registry& reg, Pools& pools)
		{
			pools[i] = PoolInfo::Of<T>(reg);
			if constexpr (sizeof...(Rest) > 1)
			{
				AddPool<i + 1, Rest...>(reg, pools);
			}
			else
			{
				pools[i + 1] = PoolInfo::Of<Rest...>(reg);
			}
		}
		template<size_t i, typename T, typename... Rest>
		static void AddPool(const char* src, Pools& pools)
		{
			pools[i] = PoolInfo::Of<T>(src);
			if constexpr (sizeof...(Rest) > 1)
			{
				AddPool<i + 1, Rest...>(src + pools[i].Size(), pools);
			}
			else
			{
				pools[i + 1] = PoolInfo::Of<Rest...>(src + pools[i].Size());
			}
		}
		static size_t CalcSize(Pools& pools)
		{
			size_t size = 0;
			for (size_t i = 0; i < pools.size(); i++)
			{
				size += pools[i].Size();
			}
			return size;
		}
	};

	template<CustomCopyable T>
	class CustomSerializer
	{
	public:
		static void SerializeAllOf(std::vector<char>& buf, ECS::Registry& _reg, Scene::Scene& scene)
		{
			auto& reg = _reg.GetUnderlyingMutable();
			auto view = reg.view<T>();
			size_t count = view.size();

			size_t startSize = buf.size();
			buf.resize(startSize + sizeof(size_t) + count * sizeof(entt::entity));

			*((size_t*)(buf.data() + startSize)) = count;
			if (count == 0)
				return;
			std::memcpy(buf.data() + startSize + sizeof(size_t), view.data(), count * sizeof(entt::entity));

			auto ids = reg.data();
			T* comps = *view.raw();
			for (size_t i = 0; i < count; i++)
			{
				Serialize(buf, comps[i], scene, ids[i]);
			}
		}
		static size_t DeserializeAllOf(const char* buf, ECS::Registry& reg, Scene::Scene& scene)
		{
			size_t count = *(reinterpret_cast<const size_t*>(buf));
			if (count == 0)
				return sizeof(size_t);

			auto ids = reinterpret_cast<const entt::entity*>(buf + sizeof(size_t));
			const char* data = reinterpret_cast<const char*>(ids + count);

			for (size_t i = 0; i < count; i++)
			{
				data += Deserialize(data, scene, ids[i]);
			}

			return data - buf;
		}
	private:
		static void Serialize(std::vector<char>& buf, T& comp, Scene::Scene& scene, ECS::Id id);
		static size_t Deserialize(const char* data, Scene::Scene& scene, ECS::Id id);
	};

	template<CustomCopyable... Types>
	class NonTrivialSerializer
	{
	public:
		static std::vector<char> Serialize(ECS::Registry& reg, Scene::Scene& scene)
		{
			std::vector<char> vec;
			Ser<Types...>(vec, reg, scene);
			return vec;
		}
		static void Deserialize(ECS::Registry& reg, const char* buf, Scene::Scene& scene)
		{
			Deser<Types...>(reg, buf, scene);
		}
	private:
		template<typename T, typename... Rest>
		static void Ser(std::vector<char>& buf, ECS::Registry& reg, Scene::Scene& scene)
		{
			CustomSerializer<T>::SerializeAllOf(buf, reg, scene);
			if constexpr (sizeof...(Rest) > 1)
			{
				Ser<Rest...>(buf, reg, scene);
			}
			else
			{
				CustomSerializer<Rest...>::SerializeAllOf(buf, reg, scene);
			}
		}
		template<typename T, typename... Rest>
		static void Deser(ECS::Registry& reg, const char* buf, Scene::Scene& scene)
		{
			auto read = CustomSerializer<T>::DeserializeAllOf(buf, reg, scene);
			if constexpr (sizeof...(Rest) > 1)
			{
				Deser<Rest...>(reg, buf + read, scene);
			}
			else
			{
				CustomSerializer<Rest...>::DeserializeAllOf(buf + read, reg, scene);
			}
		}
	};
	
	template<typename T>
	inline void iter(entt::registry& reg)
	{
		auto view0 = reg.view<T>();
		if (view0.size() == 0)
			return;
		auto ids0 = view0.data();
		auto comps0 = *view0.raw();

		Logger::Debug("Type: INT\n");
		for (size_t i = 0; i < view0.size(); i++)
		{
			Logger::Debug("  ", static_cast<uint32_t>(ids0[i]), ": ", comps0[i], "\n");
		}
	}

	class IdSerializer
	{
	public:
		static std::vector<char> Serialize(ECS::Registry& reg)
		{
			std::vector<char> buf;
			size_t count = reg.EntityCount();
			buf.resize(sizeof(size_t) + count * sizeof(entt::entity));

			*((size_t*)(buf.data())) = count;
			std::memcpy(buf.data() + sizeof(size_t), reg.EntitiesRaw(), count * sizeof(entt::entity));
		
			return buf;
		}

		static size_t Deserialize(ECS::Registry& reg, const char* data)
		{
			size_t count = *(reinterpret_cast<const size_t*>(data));

			auto ids = reinterpret_cast<const entt::entity*>(data + sizeof(size_t));
			for (size_t i = 0; i < count; i++)
				if (ids[i] != reg.CreateEntity(ids[i]))
					Logger::Error("Couldn't create entity with the rigth id.");

			return sizeof(size_t) + count * sizeof(entt::entity);
		}
	};
	
	struct BinarySerializer
	{
		template<TriviallyCopyable... Trivials>
		struct WithTrivial
		{
			template<CustomCopyable... Customes>
			struct WithCustome
			{
				using TrivialSerializer = MemcpySerializer<Trivials...>;
				using CustomSerializer = NonTrivialSerializer<Customes...>;

				static std::vector<char> Serialize(Scene::Scene& scene)
				{
					std::vector<char> ids = IdSerializer::Serialize(scene.registry);
					std::vector<char> trivials = TrivialSerializer::Serialize(scene.registry.GetUnderlyingMutable());
					std::vector<char> customes = CustomSerializer::Serialize(scene.registry, scene);
					ids.insert(ids.end(), trivials.begin(), trivials.end());
					ids.insert(ids.end(), customes.begin(), customes.end());
					return ids;
				}

				static void Deserialize(Scene::Scene& scene, const char* data)
				{
					data += IdSerializer::Deserialize(scene.registry, data);
					data += TrivialSerializer::Deserialize(scene.registry.GetUnderlyingMutable(), data);
					CustomSerializer::Deserialize(scene.registry, data, scene);
				}
			};
		};
	};

	// test
	/*using namespace Components;
	template<>
	inline void iter<Tag>(entt::registry& reg)
	{
		auto view0 = reg.view<Tag>();
		if (view0.size() == 0)
			return;
		auto ids0 = view0.data();
		auto comps0 = *view0.raw();

		Logger::Debug("Type: INT\n");
		for (size_t i = 0; i < view0.size(); i++)
		{
			Logger::Debug("  ", static_cast<uint32_t>(ids0[i]), ": ", comps0[i].tag, "\n");
		}
	}
	template<>
	inline void iter<Model>(entt::registry& reg)
	{
		auto view0 = reg.view<Model>();
		if (view0.size() == 0)
			return;
		auto ids0 = view0.data();
		auto comps0 = *view0.raw();

		Logger::Debug("Type: INT\n");
		for (size_t i = 0; i < view0.size(); i++)
		{
			Logger::Debug("  ", static_cast<uint32_t>(ids0[i]), ": ", comps0[i].meshes.size(), "\n");
		}
	}
	static void test5()
	{
		using Seri = BinarySerializer<int, float>::With<std::string, Tag, Model>;

		entt::registry reg;
		auto a = reg.create();
		auto b = reg.create();
		auto c = reg.create();
		auto d = reg.create();

		reg.emplace<Model>(a).meshes.push_back(Renderer::Data::Mesh());

		//reg.emplace<Tag>(a) = Tag{ .tag = std::string("TAG ONE") };
		//reg.emplace<Tag>(b) = Tag{ .tag = std::string("TAG WTTOWTTW") };
		//reg.emplace<Tag>(c) = Tag{ .tag = std::string("TAG 33") };

		reg.emplace<std::string>(b) = std::string("asd");
		reg.emplace<std::string>(c) = std::string("asd2");

		reg.emplace<float>(d) = 3;
		reg.emplace<float>(b) = 1;
		reg.emplace<float>(a) = 0;

		reg.emplace<int>(d) = 3;
		reg.emplace<int>(b) = 1;
		reg.emplace<int>(a) = 0;

		Logger::Debug("BEFORE SER");
		iter<Model>(reg);
		iter<Tag>(reg);
		iter<std::string>(reg);
		iter<float>(reg);
		iter<int>(reg);

		auto vec = Seri::Serialize(reg);

		reg.clear();

		Seri::Deserialize(reg, vec.data());

		Logger::Debug("AFTER SER");
		iter<Model>(reg);
		iter<Tag>(reg);
		iter<std::string>(reg);
		iter<float>(reg);
		iter<int>(reg);
	}*/
}

