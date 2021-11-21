#pragma once
#include "Components/All.h"
#include "Scene/Scene.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include "BinaryComponentSerializer.h"
#include "SpecializedBinaryComponentSerializers.h"

namespace Nork
{
	template<class... Components>
	class BinarySerializer
	{
	public:
		BinarySerializer(entt::registry& reg, Scene::Scene& scene) 
			: reg(reg), scene(scene) { }

		std::vector<char>& Serialize()
		{
			auto ids = std::span<entt::entity>((entt::entity*)reg.data(), reg.size());
			writer << ids.size();
			size_t& fullSize = writer.WriteEmpty(writer.Data().size());

			for (size_t i = 0; i < ids.size(); i++)
			{
				Serialize(ids[i]);
			}

			fullSize = writer.Data().size() - fullSize;
			return writer.Data();
		}
	private:
		void Serialize(entt::entity id)
		{
			writer << id;
			size_t& size = writer.WriteEmpty<size_t>(writer.Data().size());
			size_t& count = writer.WriteEmpty<size_t>(0);

			Serialize<Components...>(id,  count);
			
			size = writer.Data().size() - size;
		}
		template<class Comp, class... Rest>
		void Serialize(entt::entity id, size_t& count)
		{	
			Comp* comp = reg.try_get<Comp>(id);
			if (comp != nullptr)
			{
				SerializeComponentProxy(*comp, id);
				count++;
			}
			if constexpr (sizeof...(Rest) > 0)
			{
				Serialize<Rest...>(id, count);
			}
		}
		template<class Comp>
		void SerializeComponentProxy(Comp& comp, entt::entity id)
		{
			writer << componentId<Comp>;
			size_t& size = writer.WriteEmpty<size_t>();
			size = writer.Data().size();

			BinaryComponentSerializer<Comp>(reg, scene, writer, id) << comp;
			
			size = writer.Data().size() - size;
		}
	private:
		BinaryWriter writer;
		entt::registry& reg;
		Scene::Scene& scene;
	};

	template<class... Components>
	class BinaryDeserializer
	{
	public:
		BinaryDeserializer(entt::registry& reg, Scene::Scene& scene, std::span<char> data)
			: reg(reg), scene(scene), reader(data)
		{
		}

		void Deserialize()
		{
			size_t entCount, fullSize;
			reader >> entCount >> fullSize;

			for (size_t i = 0; i < entCount; i++)
			{
				DeserializeNext();
			}
		}
	private:
		void Deserialize(entt::entity id)
		{
		}
		void DeserializeNext()
		{
			entt::entity id;
			size_t entSize, compCount;
			reader >> id >> entSize >> compCount;

			auto actualId = reg.create(id);
			if (actualId != id)
			{
				Logger::Error("Couldn't create entity with id ", (size_t)id, ". Got ", (size_t)actualId, " instead");
			}

			for (size_t i = 0; i < compCount; i++)
			{
				size_t compId, compSize;
				reader >> compId >> compSize;
				auto data = reader.Read<char>(compSize);

				TryDeserializeComponent<Components...>(compId, data, id);
			}
		}
		template<class Comp, class... Rest>
		void TryDeserializeComponent(size_t compId, std::span<char> data, entt::entity id)
		{
			if (componentId<Comp> == compId)
			{
				Comp comp = Comp();
				BinaryComponentDeserializer<Comp>(reg, scene, reader, id) >> comp;
				reg.emplace<Comp>(id, comp);
			}
			else 
			{
				if constexpr (sizeof...(Rest) > 0)
				{
					TryDeserializeComponent<Rest...>(compId, data, id);
				}
			}
		}
	private:
		BinaryReader reader;
		entt::registry& reg;
		Scene::Scene& scene;
	};

	// entCount|fullSize
	// id|size|compCount
	// {compId|size|comp}...
}

