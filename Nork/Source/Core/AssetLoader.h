#pragma once

#include "Components/Drawable.h"
#include "Modules/Renderer/GLTF/gltf.h"

namespace Nork {
	namespace fs = std::filesystem;

	/*static ModelResources::ResourceType& GetCube()
	{
		static bool initialized = false;
		static fs::path gltfUri = "templates/cube_template/cube_template.gltf";
		if (!initialized)
		{
			fs::path bufUri = fs::path(gltfUri).replace_extension("");
			if (!ModelResources::Instance().Exists(gltfUri))
			{
				ModelResources::Instance().SaveAs(std::make_shared<Renderer::Model>(
					Renderer::Model{
						.meshes = { Components::Mesh {
							.mesh = { RenderingSystem::Instance().NewMesh(Renderer::MeshDataFactory::CubeVertices(), Renderer::MeshDataFactory::CubeIndices()) },
							.material = RenderingSystem::Instance().NewMaterial()
						}}
					}), gltfUri);
				initialized = true;
			}
		}
		return ModelResources::Instance().Get(gltfUri);
	}*/

	class AssetLoaderProxy;
	class AssetLoader {
	public:
		struct Exception : std::exception {
			enum class Code {
				PathDoesNotExist, UriAlreadyExists, AssetNotFoundInCache, EmptyModel, NoImporterForAsset
			};
			Code code;
			Exception(Code code, std::source_location src = std::source_location::current())
				: code(code), std::exception(CodeToString(code).append(" at ")
					.append(src.file_name()).append("::").append(src.function_name()).append(" (line ").append(std::to_string(src.line()))
					.append(")").c_str()) {}
		private:
			static std::string CodeToString(Code code) {
				switch (code)
				{ 
					using enum Code;
					case PathDoesNotExist:
						return "PathDoesNotExist";
					case UriAlreadyExists:
						return "UriAlreadyExists";
					case AssetNotFoundInCache:
						return "AssetNotFoundInCache";
					case EmptyModel:
						return "EmptyModel";
					case NoImporterForAsset:
						return "NoImporterForAsset";
					default:
						std::unreachable();
				}
			}
		};
	public:
		AssetLoader();
		std::shared_ptr<Renderer::Texture> LoadTexture(const fs::path& uri);
		std::shared_ptr<Renderer::Model> LoadModel(const fs::path& uri);
		std::shared_ptr<Renderer::Model> ImportModel(const fs::path& from, const fs::path& uri);
		void SaveModel(const std::shared_ptr<Renderer::Model>& model, const fs::path& uri);
		std::vector<fs::path> ListTemplates();
		static AssetLoaderProxy& Instance();
		fs::path UriToAbsolutePath(const fs::path& uri);
		fs::path AbsolutePathToUri(const fs::path& abs);
		fs::path ModelsPath();
		fs::path TemplatesPath();
		fs::path CubeUri();
		void SetProjectRoot(const fs::path&);
	private:
		fs::path projectAssetsRoot;
	};

	class AssetLoaderProxy: AssetLoader {
	public:
		AssetLoaderProxy();
		std::shared_ptr<Renderer::Texture> LoadTexture(const fs::path& uri);
		std::shared_ptr<Renderer::Model> LoadModel(const fs::path& uri);
		std::shared_ptr<Renderer::Model> ImportModel(const fs::path& from, std::string name = "");
		void SaveModel(const std::shared_ptr<Renderer::Model>& model);
		void ReloadModel(const std::shared_ptr<Renderer::Model>& model);
		fs::path Uri(const std::shared_ptr<Renderer::Model>& of);
		fs::path Uri(const std::shared_ptr<Renderer::Texture>& of);
		std::vector<fs::path> ListLoadedModels();
		void DeleteFromCache(const std::shared_ptr<Renderer::Model>&);
		void ClearCache();
		using AssetLoader::AbsolutePathToUri;
		using AssetLoader::CubeUri;
		void SetProjectRoot(const fs::path&);
	private:
		template<class T> fs::path Uri(const std::unordered_map<fs::path, T>& cache, const T& target) {
			for (auto& [uri, val] : cache)
				if (val == target)
					return uri;
			throw Exception(Exception::Code::AssetNotFoundInCache);
		}
	private:
		std::unordered_map<fs::path, std::shared_ptr<Renderer::Model>> models;
		std::unordered_map<fs::path, std::shared_ptr<Renderer::Texture>> textures;
	};
}