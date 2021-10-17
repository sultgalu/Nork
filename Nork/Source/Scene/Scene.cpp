#include "pch.h"
#include "Scene.h"
#include "Serialization/BinarySerializer.h"

namespace fs = std::filesystem;

namespace Nork::Scene
{
	using namespace Components;
	using namespace Serialization;
	using Serializer = BinarySerializer
		::WithTrivial<Transform,	DirLight, PointLight, DirShadow, PointShadow>
		::WithCustome<Tag, Model>;

	void Scene::Save(std::string path)
	{
		std::ofstream stream(path, std::ios::binary);
		
		auto vec = Serializer::Serialize(*this);
		stream.write(vec.data(), vec.size());
		stream.close();
	}

	void Scene::Load(std::string path)
	{
		FreeResources();
		this->registry.Wipe();
		std::ifstream stream(path, std::ios::binary | std::ios::ate);
		
		size_t size = stream.tellg();
		stream.seekg(0, std::ios_base::beg);
		std::vector<char> buf(size, '\0');
		stream.read(buf.data(), size);
		Serializer::Deserialize(*this, buf.data());
		
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
