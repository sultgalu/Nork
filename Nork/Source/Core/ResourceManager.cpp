#include "ResourceManager.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/GLTF/GLTFBuilder.h"
#include "Modules/Renderer/MeshFactory.h"
#include "RenderingSystem.h"
#include "GLTFReader.h"

namespace Nork {
	template<class T> void Resources<T>::Reload(T&& val)
	{
		auto abs = AbsolutePath(val);
		if (!fs::exists(abs))
			throw ResourceNotFoundException(abs);
		Set(val, Load(abs));
	}
	template<class T> bool Resources<T>::Exists(const fs::path& path)
	{
		return cache.contains(Relative(path)) || fs::exists(Absolute(path));
	}
	template<class T> T& Resources<T>::Get(const fs::path& path)
	{
		auto rel = Relative(path);
		if (!cache.contains(rel))
		{
			auto abs = Absolute(path);
			if (!fs::exists(abs))
				throw ResourceNotFoundException(abs);
			auto res = Load(abs);
			if (IsExternal(path))
			{ // change uri for external assets
				abs = Absolute(ImportFile(path));
				rel = Relative(abs);
			}
			cache[rel] = res;
		}
		return cache[rel];
	}
	template<class T> fs::path Resources<T>::ImportFile(const fs::path& path) const
	{
		auto extAbs = Absolute(path);
		if (!fs::exists(extAbs))
			throw ResourceNotFoundException(extAbs);

		int i = 0;
		fs::path abs = Absolute("imported") / path.filename();
		while (fs::exists(abs))
		{
			abs.replace_filename(path.stem() += " (" + std::to_string(++i) + ")").replace_extension(path.extension());
		}

		Logger::Info("Importing Asset from ", extAbs, " to ", abs);
		fs::create_directories(abs.parent_path());
		// fs::copy(extAbs, abs);
		return abs;
	}
	template<class T> fs::path Resources<T>::Uri(const T& val) const
	{
		for (auto [rel, v] : cache)
			if (v == val) return rel;
		throw ResourceNotLoadedException("Couldn't find resource in cache");
	}
	template<class T> fs::path Resources<T>::AbsolutePath(const T& val) const
	{
		return Absolute(Uri(val));
	}
	template<class T> void Resources<T>::Save(const T& val) const
	{
		Save(val, AbsolutePath(val));
	}
	template<class T> void Resources<T>::SaveAs(const T& val, const fs::path& path)
	{
		Add(val, path);
		Save(val, Absolute(path));
	}
	template<class T> bool Resources<T>::IsExternal(const fs::path& path) const
	{
		auto rel = Relative(path);
		return rel.empty() || * rel.begin() == "..";
	}
	template<class T> void Resources<T>::Add(const T& val, const fs::path& path)
	{
		auto rel = Relative(path);
		if (IsExternal(rel))
			throw ExternalLocationException(path);
		if (cache.contains(rel))
			throw FileAlreadyExistsException(rel);
		cache[rel] = val;
	}
	template<class T> fs::path Resources<T>::Absolute(const fs::path& path) const
	{
		return path.is_absolute() ? path : parentPath / path;
	}
	template<class T> fs::path Resources<T>::Relative(const fs::path& path) const
	{
		return path.is_relative() ? path : fs::relative(path, parentPath);
	}
	template<class T> Resources<T>::Resources()
	{
		parentPath = fs::current_path().append("Assets");
		fs::create_directory(parentPath);
	}
	template<class T> Resources<T>& Resources<T>::Instance()
	{
		static Resources<T> instance;
		return instance;
	}
	
	template Resources<std::shared_ptr<Components::Model>>;
	// template Resources<Renderer::Mesh>;
	// template Resources<std::shared_ptr<Renderer::Texture2D>>;

	static Renderer::GLTF::GLTF LoadGLTF(const fs::path& path)
	{
		return Renderer::GLTF::GLTF::FromJson(JsonObject::ParseFormatted(FileUtils::ReadAsString(path.string())));
	}
	static void SaveGLTF(const Renderer::GLTF::GLTF& gltf, const fs::path& path)
	{
		Logger::Info("Saving GLTF file to ", path);
		FileUtils::WriteString(gltf.ToJson().ToStringFormatted(), path.string());
	}
	static void ParseMaterial(Renderer::Material& material, const Renderer::GLTF::Material& mat, const Renderer::GLTF::GLTF& gltf)
	{
		/*material->baseColorFactor = mat.pbrMetallicRoughness.baseColorFactor;
		material->roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
		material->metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
		if (mat.pbrMetallicRoughness.baseColorTexture.Validate())
		{
			auto& texId = gltf.images[gltf.textures[mat.pbrMetallicRoughness.baseColorTexture.index].source].uri;
			material.SetTextureMap(TextureResources::Instance().Get(texId), Renderer::TextureMap::BaseColor);
		}
		if (mat.pbrMetallicRoughness.metallicRoughnessTexture.Validate())
		{
			auto& texId = gltf.images[gltf.textures[mat.pbrMetallicRoughness.metallicRoughnessTexture.index].source].uri;
			material.SetTextureMap(TextureResources::Instance().Get(texId), Renderer::TextureMap::MetallicRoughness);
		}
		if (mat.normalTexture.Validate())
		{
			auto& texId = gltf.images[gltf.textures[mat.normalTexture.index].source].uri;
			material.SetTextureMap(TextureResources::Instance().Get(texId), Renderer::TextureMap::Normal);
		}*/
	}
	/*
	template<> MeshResources::ResourceType MeshResources::Load(const fs::path& path) const
	{
		Logger::Info("Loading mesh ", path);
		std::unreachable();

		/*return RenderingSystem::Instance().world.AddMesh(
			FileUtils::ReadBinary<Renderer::Data::Vertex>((path / "vertices.bin").string()),
			FileUtils::ReadBinary<uint32_t>((path / "indices.bin").string()));
	}
	template<> TextureResources::ResourceType TextureResources::Load(const fs::path& path) const
	{
		Logger::Info("Loading texture ", path);

		auto image = Renderer::LoadUtils::LoadImage(path.string());

		std::unreachable();
		return TextureBuilder()
			.Attributes(TextureAttributes{ .width = image.width, .height = image.height, .format = image.format })
			.Params(TextureParams::Tex2DParams())
			.Create2DWithData(image.data.data());
	}*/
	template<> ModelResources::ResourceType ModelResources::Load(const fs::path& path) const
	{
		std::unreachable();
		Logger::Info("Loading model ", path);
		/*auto gltf = LoadGLTF(path);
		auto model = std::make_shared<Components::Model>();

		if (gltf.meshes.empty())
			throw GeneralResourceException(std::string("gltf file \"").append(path.string()).append("\" did not contain any models (meshes)"));
		auto& mesh = gltf.meshes.front(); // gltf::Mesh = Components::Model
		for (auto& prim : mesh.primitives)
		{ // gltf::Primitive = Components::Mesh
			const auto& indsBuf = gltf.buffers[gltf.bufferViews[gltf.accessors[prim.indices].bufferView].buffer];
			// const auto& vertsBufs = gltf.buffers[gltf.bufferViews[gltf.accessors[prim.attributes.back().accessor].bufferView].buffer]; // all of this primitive's attributes should point to the same buffer (vertices) 
			Components::Mesh mesh{ .mesh = MeshResources::Instance().Get(indsBuf.uri) };
			mesh.material = RenderingSystem::Instance().world.AddMaterial();
			if (prim.material != -1)
				ParseMaterial(mesh.material, gltf.materials[prim.material], gltf);
			model->meshes.push_back(mesh);
		}
		
		return model;*/
	}
	
	/*
	
	template<> void MeshResources::Set(MeshResources::ResourceType& dst, const MeshResources::ResourceType& src) const
	{
		std::unreachable();
	}
	template<> void TextureResources::Set(TextureResources::ResourceType& dst, const TextureResources::ResourceType& src) const
	{
		std::unreachable();
	}
	template<> void MeshResources::Save(const MeshResources::ResourceType& val, const fs::path& path) const
	{
		Logger::Info("Saving Mesh to ", path);

		fs::create_directory(path);
		FileUtils::WriteBinary(val.Vertices().Data(), val.Vertices().SizeBytes(), (path / "vertices.bin").string());
		FileUtils::WriteBinary(val.Indices().Data(), val.Indices().SizeBytes(), (path /"indices.bin").string());
	}
	template<> void TextureResources::Save(const TextureResources::ResourceType& val, const fs::path& path) const
	{
		std::unreachable();
		Logger::Info("Saving Image (", val->GetWidth(), "x", val->GetHeight(), "x",
			Renderer::TextureFormatToString(val->GetAttributes().format), ") to ", path);
		Renderer::LoadUtils::WriteTexture(*val, path.string(), Renderer::ImageFormat::BMP);
	}
	*/
	template<> void ModelResources::Set(ModelResources::ResourceType& dst, const ModelResources::ResourceType& src) const
	{
		*dst = *src;
	}

	template<> void ModelResources::Save(const ModelResources::ResourceType& val, const fs::path& path) const
	{
		Logger::Info("Saving Model to ", path);

		auto builder = Renderer::GLTFBuilder();
		std::vector<std::pair<Renderer::TextureMap, std::string>> imageUris;
		int matIdx = 0;
		builder.AddScene(true);
		for (auto& mesh : val->meshes)
		{
			// MeshResources::Instance().Save(mesh.mesh);
			auto tryAddTex = [&](Renderer::TextureMap type)
			{
				std::unreachable();
				//if (!mesh.material.HasDefault(type))
					//imageUris.push_back({ type, TextureResources::Instance().Uri(mesh.material.GetTextureMap(type)).string()});
			};
			using enum Renderer::TextureMap;
			tryAddTex(BaseColor);
			tryAddTex(MetallicRoughness);
			tryAddTex(Normal);
			// builder
			// 	.AddMesh(mesh.mesh, MeshResources::Instance().Uri(mesh.mesh).string(), matIdx++)
			// 	.AddMaterial(mesh.material, imageUris); // don't need to add if it equals the (gltf) default one
		}

		SaveGLTF(builder.Get(), path);
	}

	ModelResources::ResourceType& ResourceUtils::ImportModel(const fs::path& path)
	{
		Logger::Info("Importing model from ", path);

		std::unreachable();

		/*if (path.extension() == ".gltf")
		{
			auto model = GLTFReader(path).Read();
			ModelResources::Instance().Add(model, path.stem());
			return model;
		}
		else if (path.extension() == ".obj")
		{
			auto meshDatas = Renderer::LoadUtils::LoadModel(path.string());
			auto model = std::make_shared<Components::Model>();

			for (auto& meshData : meshDatas)
			{
				auto mesh = RenderingSystem::Instance().world.AddMesh(meshData.vertices.size(), meshData.indices.size());
				mesh.Vertices().CopyFrom(meshData.vertices.data(), meshData.vertices.size());
				mesh.Indices().CopyFrom(meshData.indices.data(), meshData.indices.size());
				MeshResources::Instance().SaveAs(mesh, fs::path(meshData.meshName).stem());

				auto material = RenderingSystem::Instance().world.AddMaterial();
				material->baseColorFactor = meshData.material.diffuse;
				material->roughnessFactor = 1 - meshData.material.specular;
				for (auto& pair : meshData.material.textureMaps)
				{
					auto tex = TextureResources::Instance().Get(pair.second);
					material.SetTextureMap(tex, pair.first);
				}

				model->meshes.push_back(Components::Mesh{ .mesh = mesh, .material = material });
			}

			ModelResources::Instance().SaveAs(model, path.stem());
			return model;
		}
		throw GeneralResourceException("no importer for " + path.extension().string());*/
	}
	void ResourceUtils::ExportModel(const ModelResources::ResourceType& model, const fs::path& path)
	{
		Logger::Info("Exporting model to ", path);
		Renderer::GLTFBuilder builder;
		int matIdx = 0;
		builder.AddScene(true);
		for (auto& meshMat : model->meshes)
		{
			// builder.AddMesh(meshMat.mesh, MeshResources::Instance().Uri(meshMat.mesh).string(), matIdx, path.parent_path());

			std::vector<std::pair<Renderer::TextureMap, std::string>> imageUris;
			auto tryAddTex = [&](Renderer::TextureMap type)
			{
				std::unreachable();
				// auto texId = TextureResources::Instance().Uri(meshMat.material.GetTextureMap(type)).string();
				// Renderer::LoadUtils::WriteTexture(*meshMat.material.GetTextureMap(type), path.string(), Renderer::ImageFormat::JPEG);
				// imageUris.push_back({ type, texId });
			};
			using enum Renderer::TextureMap;
			tryAddTex(BaseColor);
			tryAddTex(MetallicRoughness);
			tryAddTex(Normal);

			//builder.AddMaterial(meshMat.material, imageUris);
			matIdx++;
		}
		fs::create_directory(path.parent_path());
		SaveGLTF(builder.Get(), path.string());
	}
	static ModelResources::ResourceType& GetCube()
	{
		std::unreachable();
		/*static bool initialized = false;
		static fs::path gltfUri = "templates/cube_template.gltf";
		if (!initialized)
		{
			fs::path bufUri = fs::path(gltfUri).replace_extension("");
			if (!ModelResources::Instance().Exists(gltfUri))
			{
				MeshResources::Instance().SaveAs(RenderingSystem::Instance().world.AddMesh(
					Renderer::MeshFactory::CubeVertices(), Renderer::MeshFactory::CubeIndices()), bufUri);
				ModelResources::Instance().SaveAs(std::make_shared<Components::Model>(
					Components::Model{
						.meshes = { Components::Mesh {
							.mesh = { MeshResources::Instance().Get(bufUri) },
							.material = RenderingSystem::Instance().world.AddMaterial()
						}}
					}), gltfUri);
				initialized = true;
			}
		}
		return ModelResources::Instance().Get(gltfUri);*/
	}
	ModelResources::ResourceType& ResourceUtils::GetTemplate(ModelTemplate type)
	{
		fs::path path = fs::current_path().append("templates").append("models");
		switch (type)
		{
		case ModelTemplate::Cube:
			return GetCube();
		case ModelTemplate::Sphere:
			path.append("sphere").append("sphere");
			break;
		default:
			std::unreachable();
		}
		return ModelResources::Instance().Get(path.replace_extension("gltf"));
	}

	
}