#include "pch.h"
#include "Scene.h"
#include "Serialization/JsonSerializer.h"

namespace fs = std::filesystem;

namespace Nork
{
	void Scene::Save(std::string path)
	{
		std::ofstream stream(path, std::ios::out);
		
		try
		{
			stream << JsonSerializer(registry).Serialize(*root);
		} catch (std::exception e)
		{
			Logger::Error(e.what());
		}
	}
	void Scene::Load(std::string path)
	{
		registry = entt::registry();
		
		std::ifstream ifs(path, std::ios::in);
		try
		{
			std::stringstream ss;
			ss << ifs.rdbuf();
			ifs.close();
			root = JsonSerializer(registry).Deserialize(ss.str());
			// std::ofstream stream("tempmppmppmpm.json", std::ios::out);
			// try
			// {
			// 	stream << JsonSerializer(registry).Serialize(*root);
			// } catch (std::exception e)
			// {
			// 	Logger::Error(e.what());
			// }
		} catch (std::exception e)
		{
			Logger::Error(e.what());
		}
		
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
	Components::Camera& Scene::GetMainCamera()
	{
		for (auto [id, cam, tag] : registry.view<Components::Camera, Components::Tag>().each())
		{
			if (tag.tag == "Main Camera")
				return cam;
		}

		auto mainCameraNode = CreateNode();
		auto cam = mainCameraNode->GetEntity().AddComponent<Components::Camera>();
		mainCameraNode->GetEntity().AddComponent<Components::Tag>().tag = "Main Camera";
		return cam;
	}
}
