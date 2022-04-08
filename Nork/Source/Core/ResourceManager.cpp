
#include "ResourceManager.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"

#include "Modules/Renderer/GLTF/GLTFBuilder.h"

namespace Nork {
	namespace fs = std::filesystem;

	ResourceManager::ResourceManager(Renderer::DrawState& drawState)
		: drawState(drawState)
	{
		assetsPath = fs::current_path().append("Assets");
		fs::create_directory(assetsPath);
		fs::create_directory(BufferBinariesPath());
		fs::create_directory(ImagesPath());
		fs::create_directory(MeshesPath());
		fs::create_directory(MaterialsPath());
		fs::create_directory(ModelsPath());
		fs::create_directory(ExportPath());
	}

	std::string ResourceManager::ImportTexture(const std::string& extPath)
	{
		auto id = ExternalPathToAssetId(fs::path(extPath).replace_extension("bmp").string());
		auto path = AssetIdToTexturePath(id);
		if (fs::exists(path))
		{
			Logger::Error("Texture with the same path hash is already imported.");
			return id;
		}

		using namespace Renderer;
		auto image = LoadUtils::LoadImage(extPath);
		LoadUtils::WriteImage(image, path.string());
		Logger::Info("Imported texture: ", path.string(), " from ", extPath);
		return id;
	}
	Components::Model ResourceManager::ImportModel(const std::string& extPath)
	{
		auto id = ExternalPathToAssetId(extPath);
		auto path = AssetIdToModelPath(id);
		if (fs::exists(path))
		{
			Logger::Error("Model with the same path hash is already imported.");
			return GetModel(id);
		}

		ModelResource modelResource;
		auto meshDatas = Renderer::LoadUtils::LoadModel(extPath);

		for (auto& meshData : meshDatas)
		{
			auto mesh = Renderer::MeshFactory(drawState.vaoWrapper).Create(meshData.vertices, meshData.indices);
			auto material = drawState.AddMaterial();

			material->diffuse = meshData.material.diffuse;
			material->specular = meshData.material.specular;
			material->specularExponent = meshData.material.specularExponent;
			for (auto& pair : meshData.material.textureMaps)
			{
				auto texId = ImportTexture(pair.second);
				material->SetTextureMap(GetTexture(texId), pair.first);
			}
			material->Update();

			MeshResource meshResource{ .mesh = meshData.meshName, .material = meshData.materialName };
			meshes[meshResource.mesh] = mesh;
			materials[meshResource.material] = material;
			SaveMesh(mesh);
			SaveMaterial(material);
			modelResource.meshes.push_back(meshResource);
		}

		Logger::Info("Loaded model: ", extPath);
		models[id] = modelResource;
		SaveModel(modelResource, id);

		Components::Model model;
		for (auto& meshResource : models[id].meshes)
		{
			model.meshes.push_back(Components::Mesh{ .mesh = meshes[meshResource.mesh], .material = materials[meshResource.material] });
		}
		return model;
	}

	void ResourceManager::ExportModel(const Components::Model& model)
	{
		Renderer::GLTFBuilder builder(ExportPath());
		int idx = 0;
		std::vector<int> nodes;
		for (auto& meshMat : model.meshes)
		{
			builder.AddNode(idx);
			nodes.push_back(idx);

			builder.AddMesh(meshMat.mesh, *IdFor(meshMat.mesh), idx++);

			std::vector<std::pair<Renderer::TextureMap, std::string>> imageUris;
			auto tryAddTex = [&](Renderer::TextureMap type)
			{
				auto texId = IdFor(meshMat.material->GetTextureMap(type));
				if (texId.has_value())
				{
					SaveImage(meshMat.material->GetTextureMap(type), ExportPath().append(*texId).string());
					imageUris.push_back({ type, *texId });
				}
			};
			using enum Renderer::TextureMap;
			tryAddTex(Diffuse);
			tryAddTex(Roughness);
			tryAddTex(Normal);
			tryAddTex(Reflection);

			builder.AddMaterial(meshMat.material, *IdFor(meshMat.material), imageUris);
		}
		builder.AddScene(nodes, true);
		SaveGLTF(builder.Get(), ExportPath().append(*IdForModel(model.meshes[0].mesh)).replace_extension("gltf").string());
	}

	Components::Model ResourceManager::GetModelByPath(const std::string& path)
	{
		return GetModel(fs::path(path).filename().string());
	}
	Components::Model ResourceManager::GetModel(const std::string& id)
	{
		auto opt = models.find(id);
		if (opt == models.end())
		{
			LoadModel(id);
		}
		Components::Model model;
		for (auto& meshResource : models[id].meshes)
		{
			model.meshes.push_back(Components::Mesh{ .mesh = meshes[meshResource.mesh], .material = materials[meshResource.material] });
		}
		return model;
	}
	std::shared_ptr<Renderer::Mesh> ResourceManager::GetMesh(const std::string& id)
	{
		auto opt = meshes.find(id);
		if (opt == meshes.end())
		{
			LoadMesh(id);
		}
		return meshes[id];
	}
	std::shared_ptr<Renderer::Texture2D> ResourceManager::GetTextureByPath(const std::string& path)
	{
		return GetTexture(fs::path(path).filename().string());
	}
	std::shared_ptr<Renderer::Material> ResourceManager::GetMaterial(const std::string& id)
	{
		auto opt = materials.find(id);
		if (opt == materials.end())
		{
			LoadMaterial(id);
		}
		return materials[id];
	}
	std::shared_ptr<Renderer::Texture2D> ResourceManager::GetTexture(const std::string& id)
	{
		auto opt = textures.find(id);
		if (opt == textures.end())
		{
			LoadTexture(id);
		}
		return textures[id];
	}

	void ResourceManager::LoadTexture(const std::string& id)
	{
		using namespace Renderer;
		
		auto path = AssetIdToTexturePath(id);
		if (!fs::exists(path))
		{
			Logger::Error("Texture path does not exists: ", path.string());
			return;
		}

		auto image = LoadUtils::LoadImage(path.string());
		auto tex = TextureBuilder()
			.Attributes(TextureAttributes{ .width = image.width, .height = image.height, .format = image.format })
			.Params(TextureParams::Tex2DParams())
			.Create2DWithData(image.data.data());
		textures[id] = tex;
		Logger::Info("Loaded texture ", id);
	}
	void ResourceManager::LoadMesh(const std::string& id)
	{
		auto gltf = LoadGLTF(AssetIdToMeshPath(id).string());
		if (gltf.meshes.empty() || gltf.meshes.back().primitives.empty())
		{
			Logger::Error("gltf file ", AssetIdToMeshPath(id).string(), " did not contain any meshes");
			return;
		}

		auto indsBufs = gltf.buffers[
			gltf.bufferViews[
				gltf.accessors[
					gltf.meshes.back().primitives.back().indices
				].bufferView
			].buffer
		]; 
		auto vertsBufs = gltf.buffers[
			gltf.bufferViews[
				gltf.accessors[
					gltf.meshes.back().primitives.back().attributes.back().accessor
				].bufferView
			].buffer
		];

		auto mesh = Renderer::MeshFactory(drawState.vaoWrapper).Create(
			FileUtils::ReadBinary<Renderer::Data::Vertex>(MeshesPath().append(vertsBufs.uri).string(), vertsBufs.byteLength),
			FileUtils::ReadBinary<uint32_t>(MeshesPath().append(indsBufs.uri).string(), indsBufs.byteLength));

		meshes[id] = mesh;
		Logger::Info("Loaded mesh ", id);
	}
	void ResourceManager::LoadMaterial(const std::string& id)
	{
		auto gltf = LoadGLTF(AssetIdToMaterialPath(id).string());
		if (gltf.materials.empty())
		{
			Logger::Error("gltf file ", AssetIdToMeshPath(id).string(), " did not contain any materials");
			return;
		}

		auto material = drawState.AddMaterial();
		auto& mat = gltf.materials.back();

		material->diffuse = mat.pbrMetallicRoughness.baseColorFactor;
		material->specular = 1 - mat.pbrMetallicRoughness.roughnessFactor;
		material->specularExponent = mat.pbrMetallicRoughness.extras.Get<float>("specularExponent");
		if (mat.pbrMetallicRoughness.baseColorTexture.Validate())
		{
			auto texId = gltf.images[mat.pbrMetallicRoughness.baseColorTexture.index].uri;
			material->SetTextureMap(GetTexture(texId), Renderer::TextureMap::Diffuse);
		}
		if (mat.pbrMetallicRoughness.metallicRoughnessTexture.Validate())
		{
			auto texId = gltf.images[mat.pbrMetallicRoughness.metallicRoughnessTexture.index].uri;
			material->SetTextureMap(GetTexture(texId), Renderer::TextureMap::Roughness);
		}
		if (mat.normalTexture.Validate())
		{
			auto texId = gltf.images[mat.normalTexture.index].uri;
			material->SetTextureMap(GetTexture(texId), Renderer::TextureMap::Normal);
		}
		material->Update();

		materials[id] = material;
		Logger::Info("Loaded material ", id);
	}
	void ResourceManager::LoadModel(const std::string& id)
	{
		if (id == "")
		{
			auto defaultMesh = Renderer::MeshFactory(drawState.vaoWrapper).CreateCube();
			auto defaultMaterial = drawState.AddMaterial();
			meshes[""] = defaultMesh;
			materials[""] = defaultMaterial;
			models[""].meshes = { MeshResource {.mesh = "", .material = "" } };
			return;
		}

		auto path = AssetIdToModelPath(id);
		if (!fs::exists(path))
		{
			Logger::Error("Path ", path, " does not exist for Asset ", id);
			return;
		}
		ModelResource model;
		auto json = JsonObject::Parse(FileUtils::ReadAsString(path.string()));
		for (auto& obj : json.Get<JsonArray>("meshes").Get<JsonObject>())
		{
			auto meshResource = MeshResource{
				.mesh = obj.Get<std::string>("mesh"),
				.material = obj.Get<std::string>("material")
			};
			model.meshes.push_back(meshResource);
			GetMaterial(meshResource.material); // ensure is loaded
			GetMesh(meshResource.mesh);
		}
		models[id] = model;
		Logger::Info("Loaded model ", id);
	}

	void ResourceManager::SaveModel(const ModelResource& model, const std::string& id)
	{
		JsonArray array;
		for (auto& mesh : model.meshes)
		{
			array.Element<JsonObject>(JsonObject()
				.Property("mesh", mesh.mesh)
				.Property("material", mesh.material));
		}
		auto json = JsonObject().Property("meshes", array);
		std::ofstream file(AssetIdToModelPath(id).string());
		file << json.ToString();
	}
	void ResourceManager::SaveMaterial(std::shared_ptr<Renderer::Material> material)
	{
		auto path = PathFor(material);
		if (!path.has_value())
		{
			Logger::Error("Failed to Serialized Renderer::Material, bacause it's path is not found");
			return;
		}
		std::vector<std::pair<Renderer::TextureMap, std::string>> imageUris;
		auto tryAddTex = [&](Renderer::TextureMap type)
		{
			auto tex = IdFor(material->GetTextureMap(type));
			if (tex.has_value())
			{
				imageUris.push_back({ type, *tex });
			}
		};
		using enum Renderer::TextureMap;
		tryAddTex(Diffuse);
		tryAddTex(Roughness);
		tryAddTex(Normal);
		tryAddTex(Reflection);

		auto gltf = Renderer::GLTFBuilder(BufferBinariesPath())
			.AddMaterial(material, *IdFor(material), imageUris)
			.Get();
		SaveGLTF(gltf, *path);
	}
	void ResourceManager::SaveMesh(std::shared_ptr<Renderer::Mesh> mesh)
	{
		auto path = PathFor(mesh);
		if (!path.has_value())
		{
			Logger::Error("Failed to Serialized Renderer::Mesh, bacause it's path is not found");
			return;
		}
		auto gltf = Renderer::GLTFBuilder(BufferBinariesPath())
			.AddMesh(mesh, *IdFor(mesh))
			.Get();
		SaveGLTF(gltf, *path);
	}
	void ResourceManager::SaveImage(std::shared_ptr<Renderer::Texture2D> tex, const std::string& path)
	{
		Renderer::Image image;
		image.format = tex->GetAttributes().format;
		image.width = tex->GetWidth();
		image.height = tex->GetHeight();
		image.channels = Renderer::GetTextureChannelCount(image.format);
		image.data.resize(Renderer::GetTexturePixelSize(image.format) * image.width * image.height, 0);
		tex->Bind().GetData2D(image.data.data());
		Renderer::LoadUtils::WriteImage(image, path);
		Logger::Info("Saved Image (", image.width, "x", image.height, "x", image.channels, ", ", Renderer::TextureFormatToString(image.format), ") to ", path);
	}
	void ResourceManager::SaveGLTF(const Renderer::GLTF::GLTF& gltf, const std::string& path)
	{
		FileUtils::WriteString(gltf.ToJson().ToString(), path);
		Logger::Info("Saved GLTF file to ", path);
	}

	Renderer::GLTF::GLTF ResourceManager::LoadGLTF(const std::string& path)
	{
		return Renderer::GLTF::GLTF::FromJson(JsonObject::Parse(FileUtils::ReadAsString(path)));
	}

	std::optional<std::string> ResourceManager::PathFor(std::shared_ptr<Renderer::Texture2D> texturePtr)
	{
		for (auto [id, ptr] : textures)
		{
			if (texturePtr == ptr)
			{
				return AssetIdToTexturePath(id).string();
			}
		}
		return std::nullopt;
	}
	std::optional<std::string> ResourceManager::PathFor(std::shared_ptr<Renderer::Material> materialPtr)
	{
		for (auto [id, ptr] : materials)
		{
			if (materialPtr == ptr)
			{
				return AssetIdToMaterialPath(id).string();
			}
		}
		return std::nullopt;
	}
	std::optional<std::string> ResourceManager::PathFor(std::shared_ptr<Renderer::Mesh> meshPtr)
	{
		for (auto [id, ptr] : meshes)
		{
			if (meshPtr == ptr)
			{
				return AssetIdToMeshPath(id).string();
			}
		}
		return std::nullopt;
	}
	std::optional<std::string> ResourceManager::PathForModel(std::shared_ptr<Renderer::Mesh> meshPtr)
	{
		for (auto [id, model] : models)
		{
			if (meshes[model.meshes.back().mesh] == meshPtr)
			{
				return AssetIdToModelPath(id).string();
			}
		}
		return std::nullopt;
	}
	std::optional<std::string> ResourceManager::IdFor(std::shared_ptr<Renderer::Texture2D> texturePtr)
	{
		for (auto [id, ptr] : textures)
		{
			if (texturePtr == ptr)
			{
				return id;
			}
		}
		return std::nullopt;
	}
	std::optional<std::string> ResourceManager::IdFor(std::shared_ptr<Renderer::Material> materialPtr)
	{
		for (auto [id, ptr] : materials)
		{
			if (materialPtr == ptr)
			{
				return id;
			}
		}
		return std::nullopt;
	}
	std::optional<std::string> ResourceManager::IdFor(std::shared_ptr<Renderer::Mesh> meshPtr)
	{
		for (auto [id, ptr] : meshes)
		{
			if (meshPtr == ptr)
			{
				return id;
			}
		}
		return std::nullopt;
	}
	std::optional<std::string> ResourceManager::IdForModel(std::shared_ptr<Renderer::Mesh> meshPtr)
	{
		for (auto [id, model] : models)
		{
			if (meshes[model.meshes.back().mesh] == meshPtr)
			{
				return id;
			}
		}
		return std::nullopt;
	}

	std::filesystem::path ResourceManager::AssetsPath()
	{
		return std::filesystem::path(assetsPath);
	}
	std::filesystem::path ResourceManager::ModelsPath()
	{
		return AssetsPath().append("models");
	}
	std::filesystem::path ResourceManager::MeshesPath()
	{
		return AssetsPath().append("meshes");
	}
	std::filesystem::path ResourceManager::BufferBinariesPath()
	{
		return AssetsPath().append("meshes");
	}
	std::filesystem::path ResourceManager::MaterialsPath()
	{
		return AssetsPath().append("materials");
	}
	std::filesystem::path ResourceManager::ImagesPath()
	{
		return AssetsPath().append("materials");
	}
	std::filesystem::path ResourceManager::ExportPath()
	{
		return AssetsPath().append("export");
	}

	std::string ResourceManager::ExternalPathToAssetId(const std::string& path)
	{
		std::string dir = path.substr(0, path.find_last_of("\\/") - 2);
		auto dirHash = std::hash<std::string>()(dir);
		return std::to_string(dirHash) + "_" + path.substr(path.find_last_of("\\/") + 1);
	}
	std::filesystem::path ResourceManager::AssetIdToTexturePath(const std::string& id)
	{
		return ImagesPath().append(id).replace_extension("bmp");
	}
	std::filesystem::path ResourceManager::AssetIdToMeshPath(const std::string& id)
	{
		return MeshesPath().append(id).replace_extension("gltf");
	}
	std::filesystem::path ResourceManager::AssetIdToMaterialPath(const std::string& id)
	{
		return ImagesPath().append(id).replace_extension("gltf");
	}
	std::filesystem::path ResourceManager::AssetIdToModelPath(const std::string& id)
	{
		return ModelsPath().append(id).replace_extension("json");
	}
}