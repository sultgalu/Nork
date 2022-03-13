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

namespace Nork
{
	void Scene::Save(std::string path)
	{
		std::ofstream stream(path, std::ios::binary);
		
		auto vec = Serializer(registry, *this).Serialize();
		stream.write(vec.data(), vec.size());
		stream.close();
	}
	void Scene::Load(std::string path)
	{
		registry = entt::registry();
		std::ifstream stream(path, std::ios::binary | std::ios::ate);
		
		size_t size = stream.tellg();
		stream.seekg(0, std::ios_base::beg);
		std::vector<char> buf(size, '\0');
		stream.read(buf.data(), size);
		Deserializer(registry, *this, std::span(buf)).Deserialize();
		
		stream.close();
	}
	Scene::Scene() 
		: registry(), root(SceneNode::CreateRoot(Entity(registry.create(), registry)))
	{
		mainCameraNode = CreateNode();
		mainCameraNode->GetEntity().AddComponent<Components::Camera>();
		root.GetEntity().AddComponent<Components::Tag>().tag = "root";
	}
	std::shared_ptr<SceneNode> Scene::CreateNode()
	{
		Entity entity(registry.create(), registry);
		return root.AddChild(entity);
	}
	std::shared_ptr<SceneNode> Scene::CreateNode(SceneNode& parent)
	{
		Entity entity(registry.create(), registry);
		return parent.AddChild(entity);
	}
	void Scene::DeleteNode(SceneNode& node)
	{
		for (size_t i = 0; i < node.GetChildren().size(); i++)
		{
			node.GetParent().AddChild(node.GetChildren()[i]);
		}
		registry.destroy(node.GetEntity().Id());
		node.GetParent().RemoveChild(node);
	}
}
