#include "pch.h"
#include "Scene.h"
#include "Serialization/Serializer.h"

namespace fs = std::filesystem;

namespace Nork::Scene
{
	void Scene::Save(std::string path)
	{
		std::ofstream file(path, std::ios::binary);
		Serialization::SerializeRegistry(this->registry, file);
		file.close();
	}

	void Scene::Load(std::string path)
	{
		std::string dirName = path.substr(path.find_last_of('\\') + 1, path.find_last_of('.') - (path.find_last_of('\\') + 1));
		std::ifstream refFile(path, std::ios::binary);
		this->registry.Clear();
		Serialization::DeserializeRegistry(this->registry, refFile);
		refFile.close();
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
