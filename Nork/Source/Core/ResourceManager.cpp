
#include "ResourceManager.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"

namespace Nork {
	namespace fs = std::filesystem;

	ResourceManager::ResourceManager(Renderer::DrawState& drawState)
		: drawState(drawState)
	{
		assetsPath = fs::current_path().append("Assets");
		fs::create_directory(assetsPath);
	}
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
	ResourceRef<Renderer::Texture2D> ResourceManager::GetTexture(const std::string& path)
	{
		auto id = TexturePathToAssetPath(path).string();
		auto opt = textures.find(id);
		if (opt == textures.end())
		{
			auto shared = LoadTexture(path);
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
			auto defaultMesh = std::make_shared<Resource<Renderer::Mesh>>(id, Renderer::MeshFactory(drawState.vaoWrapper).CreateCube());
			auto defaultMaterial = std::make_shared<Resource<Renderer::Material>>(id, drawState.AddMaterial());
			meshes[""] = defaultMesh;
			materials[""] = defaultMaterial;
			models[""] = { { "", "" } };
			return;
		}

		std::vector<std::pair<std::string, std::string>> modelStrings;
		std::vector<std::pair<ResourceRef<Renderer::Mesh>, ResourceRef<Renderer::Material>>> model;
		auto meshDatas = Renderer::LoadUtils::LoadModel(id);
		for (auto& meshData : meshDatas)
		{
		}

		for (auto& meshData : meshDatas)
		{
			auto mesh = Renderer::MeshFactory(drawState.vaoWrapper).Create(meshData.vertices, meshData.indices);
			auto material = drawState.AddMaterial();
			
			material->diffuse = meshData.material.diffuse;
			material->specular = meshData.material.specular;
			material->specularExponent = meshData.material.specularExponent;
			for (auto& pair : meshData.material.textureMaps)
			{
				material->SetTextureMap(GetTexture(pair.second)->object, pair.first);
			}
			material->Update();

			auto meshResRef = std::make_shared<Resource<Renderer::Mesh>>(id + "_" + meshData.meshName, mesh);
			auto materialResRef = std::make_shared<Resource<Renderer::Material>>(id + "_" + meshData.materialName, material);
			meshes[meshResRef->id] = meshResRef;
			materials[materialResRef->id] = materialResRef;
			modelStrings.push_back({ meshResRef->id, materialResRef->id });
		}

		Logger::Info("Loaded model: ", id);
		models[id] = modelStrings;
	}
	std::shared_ptr<Resource<Renderer::Texture2D>> ResourceManager::LoadTexture(const std::string& path)
	{
		using namespace Renderer;
		
		auto id = TexturePathToAssetPath(path);
		Image image;
		if (fs::exists(id))
		{
			image = LoadUtils::LoadImage(id.string());
		}
		else
		{
			image = LoadUtils::LoadImage(path);
			LoadUtils::WriteImage(image, id.string());
		}

		auto tex = TextureBuilder()
			.Attributes(TextureAttributes{ .width = image.width, .height = image.height, .format = image.format })
			.Params(TextureParams::Tex2DParams())
			.Create2DWithData(image.data.data());
		Logger::Info("Loaded texture: ", id);
		return std::make_shared<Resource<Renderer::Texture2D>>(id.string(), tex);
	}
	std::filesystem::path ResourceManager::TexturePathToAssetPath(std::string path)
	{
		for (auto& c : path)
		{
			if (c == '\\' || c == '/' || c == ':')
				c = '_';
		}
		auto fn = path.substr(path.find_last_of("\\/") + 1);
		auto id = assetsPath;
		id.append(fn).replace_extension("bmp");
		return id;
	}
}