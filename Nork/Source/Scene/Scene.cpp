#include "pch.h"
#include "Scene.h"
#include "Serialization/JsonSerializer.h"

namespace fs = std::filesystem;

namespace Nork
{
void Scene::Save() {
	SaveAs(sceneUri.string());
}
void Scene::SaveAs(std::string path)
{
	std::ofstream stream(path, std::ios::out);

	try
	{
		stream << JsonSerializer(registry).Serialize(*root);
	} catch (std::exception e)
	{
		Logger::Error(e.what());
	}
	sceneUri = path;
}
void Scene::Load(std::string path)
{
	//registry = entt::registry();
	registry.clear();
	std::ifstream ifs(path, std::ios::in);
	try
	{
		std::stringstream ss;
		ss << ifs.rdbuf();
		ifs.close();
		root = JsonSerializer(registry).Deserialize(ss.str());
	} catch (std::exception e)
	{
		Logger::Error(e.what());
	}
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
