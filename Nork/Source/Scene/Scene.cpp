#include "pch.h"
#include "Scene.h"
#include "Serialization/JsonSerializer.h"

namespace fs = std::filesystem;

namespace Nork
{
struct NodeReferenceComponent { // only runtime
	std::weak_ptr<SceneNode> node;
};

void Scene::Save() {
	if (sceneUri.empty()) {
		Logger::Warning("Cannot save scene, no sceneUri");
		return;
	}
	SaveAs(sceneUri.string());
}
void Scene::Serialize(std::ostream& os)
{
	try
	{
		os << JsonSerializer(registry).Serialize(*root);
	} catch (std::exception e)
	{
		Logger::Error(e.what());
	}
}
void Scene::Create(const fs::path& path)
{
	registry.clear();
	root = CreateRoot(registry);
	root->GetEntity().AddComponent<Components::Tag>().tag = "root";
	auto defaultCube = CreateNode()->GetEntity();
	defaultCube.AddComponent<Components::Tag>().tag = "cube";
	defaultCube.AddComponent<Components::Transform>();
	defaultCube.AddComponent<Components::Drawable>();
	auto light = CreateNode()->GetEntity();
	light.AddComponent<Components::Tag>().tag = "light";
	light.AddComponent<Components::DirLight>();
	SaveAs(path.string());
}
void Scene::Deserialize(const std::istream& is)
{
	//registry = entt::registry();
	registry.clear();
	try
	{
		std::stringstream ss;
		ss << is.rdbuf();
		root = JsonSerializer(registry).Deserialize(ss.str());
	} catch (std::exception e)
	{
		Logger::Error(e.what());
	}
}
void Scene::SaveAs(const fs::path& path)
{
	std::ofstream ofs(path, std::ios::out);
	Serialize(ofs);
	sceneUri = path;
}
void Scene::Load(const fs::path& path)
{
	Deserialize(std::ifstream(path, std::ios::in));
	sceneUri = path;
}
std::shared_ptr<SceneNode> Scene::CreateNode()
{
	return CreateNode(*root);
}
std::shared_ptr<SceneNode> Scene::CreateNode(SceneNode& parent)
{
	Entity entity(registry.create(), registry);
	auto node = parent.AddChild(entity);
	entity.AddComponent<NodeReferenceComponent>().node = node;
	return node;
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
std::shared_ptr<SceneNode> Scene::GetNodeById(entt::entity id)
{
	return registry.get<NodeReferenceComponent>(id).node.lock();
}
Scene::Scene()
	: registry(), root(CreateRoot(registry))
{
	root->GetEntity().AddComponent<Components::Tag>().tag = "root";
}
std::shared_ptr<SceneNode> Scene::CreateRoot(entt::registry& registry)
{
	return std::make_shared<SceneNode>(SceneNode(Entity(registry.create(), registry)));
}
}
