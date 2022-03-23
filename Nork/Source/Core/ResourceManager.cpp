
#include "ResourceManager.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"

namespace Nork {
	ResourceRef<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>> ResourceManager::GetMeshes(const std::string& id)
	{
		auto opt = meshes.find(id);
		if (opt == meshes.end())
		{
			auto shared = LoadMeshes(id);
			meshes[id] = shared;
			return shared;
		}
		return meshes[id];
	}
	ResourceRef<Renderer::Texture2D> ResourceManager::GetTexture(const std::string& id)
	{
		auto opt = textures.find(id);
		if (opt == textures.end())
		{
			auto shared = LoadTexture(id);
			textures[id] = shared;
			return shared;
		}
		return textures[id];
	}
	ResourceRef<Renderer::Material> ResourceManager::GetMaterial(const std::string& id)
	{
		auto opt = materials.find(id);
		if (opt == materials.end())
		{
			auto shared = LoadMaterial(id);
			materials[id] = shared;
			return shared;
		}
		return materials[id];
	}
	ResourceRef<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>> ResourceManager::LoadMeshes(const std::string& id)
	{
		if (id == "")
		{
			return std::make_shared<Resource<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>>>(id, std::make_shared<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>>(std::vector<std::pair<Renderer::Mesh, Renderer::Material>> { { Renderer::Mesh::Cube(), Renderer::Material() } }));
		}
		auto meshDatas = Renderer::LoadUtils::LoadModel(id);
		auto shared = std::make_shared<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>>();
		for (auto& meshData : meshDatas)
		{
			Renderer::Material material;
			material.diffuse = meshData.material.diffuse;
			material.specular = meshData.material.specular;
			material.specularExponent = meshData.material.specularExponent;
			for (auto& pair : meshData.material.textureMaps)
			{
				using enum Renderer::TextureMap;
				switch (pair.first)
				{
				case Diffuse:
					material.diffuseMap = GetTexture(pair.second)->object;
					break;
				case Normal:
					material.normalsMap = GetTexture(pair.second)->object;
					break;
				case Roughness:
					material.roughnessMap = GetTexture(pair.second)->object;
					break;
				case Reflection:
					material.reflectMap = GetTexture(pair.second)->object;
					break;
				}
			}

			if (meshData.indices.size() > 0)
				shared->push_back({ Renderer::Mesh(meshData.vertices, meshData.indices), material });
			else
				shared->push_back({ Renderer::Mesh(meshData.vertices), material });
		}
		Logger::Info("Loaded model: ", id);
		return std::make_shared<Resource<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>>>(id, shared);
	}
	std::shared_ptr<Resource<Renderer::Texture2D>> ResourceManager::LoadTexture(const std::string& id)
	{
		using namespace Renderer;
		auto image = LoadUtils::LoadImage(id);
		auto tex = TextureBuilder()
			.Attributes(TextureAttributes{ .width = image.width, .height = image.height, .format = image.format })
			.Params(TextureParams::Tex2DParams())
			.Create2DWithData(image.data.data());
		Logger::Info("Loaded texture: ", id);
		return std::make_shared<Resource<Renderer::Texture2D>>(id, tex);
	}
	ResourceRef<Renderer::Material> ResourceManager::LoadMaterial(const std::string& id)
	{
		// if (id == "")
		// {
		// 	return std::make_shared<Resource<Renderer::Material>>(Renderer::Material());
		// }
		std::abort();
	}
}