
#include "ResourceManager.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"

namespace Nork {
	std::vector<std::pair<ResourceRef<Renderer::Mesh>, ResourceRef<Renderer::Material>>> ResourceManager::GetModel(const std::string& id)
	{
		auto opt = models.find(id);
		if (opt == models.end())
		{
			LoadModel(id);
		}
		std::vector<std::pair<ResourceRef<Renderer::Mesh>, ResourceRef<Renderer::Material>>> result;
		for (auto [meshId, materialId] : models[id])
		{
			result.push_back({ meshes[meshId], materials[materialId] });
		}
		return result;
	}
	ResourceRef<Renderer::Mesh> ResourceManager::GetMesh(const std::string& id)
	{
		auto opt = meshes.find(id);
		if (opt == meshes.end())
		{
			std::abort();
			// auto shared = LoadMeshes(id);
			// meshes[id] = shared;
			// return shared;
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
			std::abort();
			// auto shared = LoadMaterial(id);
			// materials[id] = shared;
			// return shared;
		}
		return materials[id];
	}
	void ResourceManager::LoadModel(const std::string& id)
	{
		if (id == "")
		{
			auto defaultMesh = std::make_shared<Resource<Renderer::Mesh>>(id, meshStorage.AddCube());
			auto defaultMaterial = std::make_shared<Resource<Renderer::Material>>(id, materialStorage.Add());
			meshes[""] = defaultMesh;
			materials[""] = defaultMaterial;
			models[""] = { { "", "" } };
			return;
		}

		std::vector<std::pair<std::string, std::string>> modelStrings;
		std::vector<std::pair<ResourceRef<Renderer::Mesh>, ResourceRef<Renderer::Material>>> model;
		auto meshDatas = Renderer::LoadUtils::LoadModel(id);
		std::vector<std::pair<const std::vector<Renderer::Model::Vertex>&, const std::vector<uint32_t>&>> verticiesIndicies;
		for (auto& meshData : meshDatas)
		{
			verticiesIndicies.push_back({ meshData.vertices, meshData.indices });
		}
		auto newMeshes = meshStorage.Add(verticiesIndicies);
		auto newMaterials = materialStorage.Add(meshDatas.size());

		int count = -1;
		for (auto& meshData : meshDatas)
		{
			count++;
			auto mesh = newMeshes[count]; //meshStorage.Add(meshData.vertices, meshData.indices);
			auto material = newMaterials[count]; //materialStorage.Add();

			material->diffuse = meshData.material.diffuse;
			material->specular = meshData.material.specular;
			material->specularExponent = meshData.material.specularExponent;
			for (auto& pair : meshData.material.textureMaps)
			{
				using enum Renderer::TextureMap;
				switch (pair.first)
				{
				case Diffuse:
					material->diffuseMap = GetTexture(pair.second)->object;
					break;
				case Normal:
					material->normalsMap = GetTexture(pair.second)->object;
					break;
				case Roughness:
					material->roughnessMap = GetTexture(pair.second)->object;
					break;
				case Reflection:
					material->reflectMap = GetTexture(pair.second)->object;
					break;
				}
			}
			materialStorage.Update(material);

			auto meshResRef = std::make_shared<Resource<Renderer::Mesh>>(id + "_" + meshData.meshName, mesh);
			auto materialResRef = std::make_shared<Resource<Renderer::Material>>(id + "_" + meshData.materialName, material);
			meshes[meshResRef->id] = meshResRef;
			materials[materialResRef->id] = materialResRef;
			modelStrings.push_back({ meshResRef->id, materialResRef->id });
		}

		Logger::Info("Loaded model: ", id);
		models[id] = modelStrings;
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
}