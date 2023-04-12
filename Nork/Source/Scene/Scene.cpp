#include "pch.h"
#include "Scene.h"
#include "Serialization/JsonSerializer.h"

namespace fs = std::filesystem;

namespace Nork
{
void Scene::Save() {
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
void Scene::SaveAs(std::string path)
{
	std::ofstream ofs(path, std::ios::out);
	Serialize(ofs);
	sceneUri = path;
}
void Scene::Load(std::string path)
{
	Deserialize(std::ifstream(path, std::ios::in));
	sceneUri = path;
}
Scene::Scene()
	: registry(), root(std::make_shared<SceneNode>(SceneNode(Entity(registry.create(), registry))))
{
	root->GetEntity().AddComponent<Components::Tag>().tag = "root";
}
std::shared_ptr<SceneNode> Scene::CreateNode()
{
	Entity entity(registry.create(), registry);
	return root->AddChild(entity);
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
