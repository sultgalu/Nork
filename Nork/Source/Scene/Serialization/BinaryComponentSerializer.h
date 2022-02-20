#pragma once

namespace Nork
{
	template<class T>
	static constexpr size_t componentId;

	template<class Component>
	class BinaryComponentSerializer
	{
	public:
		BinaryComponentSerializer(entt::registry& reg, Scene& scene, BinaryWriter& writer, entt::entity id)
			: reg(reg), scene(scene), writer(writer), id(id)
		{
		}

		void operator<<(Component& comp)
		{
			writer << comp;
		}
	private:
		BinaryWriter& writer;
		entt::registry& reg;
		Scene& scene;
		entt::entity id;
	};

	template<class Component>
	class BinaryComponentDeserializer
	{
	public:
		BinaryComponentDeserializer(entt::registry& reg, Scene& scene, BinaryReader& reader, entt::entity id)
			: reg(reg), scene(scene), reader(reader), id(id)
		{
		}
		void operator>>(Component& comp)
		{
			comp = *((Component*)reader.Data().data());
		}
	private:
		BinaryReader& reader;
		entt::registry& reg;
		Scene& scene;
		entt::entity id;
	};

}