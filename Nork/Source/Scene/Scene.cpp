#include "pch.h"
#include "Scene.h"
#include "Serialization/BinarySerializer.h"

namespace fs = std::filesystem;

namespace Nork
{
	using namespace Components;

	template<> static constexpr size_t componentId<Transform> = 0;
	template<> static constexpr size_t componentId<DirLight> = 1;
	template<> static constexpr size_t componentId<PointLight> = 2;
	template<> static constexpr size_t componentId<DirShadow> = 3;
	template<> static constexpr size_t componentId<PointShadow> = 4;
	template<> static constexpr size_t componentId<Kinematic> = 5;
	template<> static constexpr size_t componentId<Tag> = 6;
	template<> static constexpr size_t componentId<Model> = 7;

	template<class... Components>
	struct SerializationRegistry
	{
		using BinarySerializer = BinarySerializer<Components...>;
		using BinaryDeserializer = BinaryDeserializer<Components...>;
	};
	using SerializationFactory =
		SerializationRegistry<Transform, DirLight, PointLight,
		DirShadow, PointShadow, Kinematic, Tag, Model>;

	using Serializer = SerializationFactory::BinarySerializer;
	using Deserializer = SerializationFactory::BinaryDeserializer;
}

namespace Nork::Scene
{
	void Scene::Save(std::string path)
	{
		std::ofstream stream(path, std::ios::binary);
		
		auto vec = Serializer(registry.GetUnderlyingMutable(), *this).Serialize();
		stream.write(vec.data(), vec.size());
		stream.close();
	}

	void Scene::Load(std::string path)
	{
		FreeResources();
		registry.Wipe();
		std::ifstream stream(path, std::ios::binary | std::ios::ate);
		
		size_t size = stream.tellg();
		stream.seekg(0, std::ios_base::beg);
		std::vector<char> buf(size, '\0');
		stream.read(buf.data(), size);
		Deserializer(registry.GetUnderlyingMutable(), *this, std::span(buf)).Deserialize();
		
		stream.close();
	}

	Components::Model Nork::Scene::Scene::GetModelByResource(std::vector<Renderer::Data::MeshResource> resource)
	{
		Components::Model model;
		model.meshes.reserve(resource.size());
		for (size_t i = 0; i < resource.size(); i++)
		{
			model.meshes.push_back(Renderer::Data::Mesh(resource[i]));
		}
		return model;
	}
	uuid Scene::GenUniqueId()
	{
		return std::rand();
	}
}
