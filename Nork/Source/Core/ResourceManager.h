#pragma once

#include "Components/Drawable.h"
#include "Modules/Renderer/GLTF/gltf.h"

namespace Nork {
	namespace fs = std::filesystem;

	struct ResourcesException : std::exception
	{
		ResourcesException(std::string_view type, std::string_view reason)
			: std::exception(std::string(type).append(": ").append(reason).c_str()) {}
	};
	struct GeneralResourceException : ResourcesException
	{
		GeneralResourceException(std::string_view reason)
			: ResourcesException("GeneralResourceException", reason)
		{}
	};
	struct ResourceNotFoundException : ResourcesException
	{
		ResourceNotFoundException(const fs::path& path)
			: ResourcesException("ResourceNotFoundException", std::string("Couldn't find resource at path: ").append(path.string())) {}
	};
	struct ResourceNotLoadedException : ResourcesException
	{
		ResourceNotLoadedException(const char* reason)
			: ResourcesException("ResourceNotLoadedException", reason)
		{}
	};
	struct IdNotFoundException : ResourcesException
	{
		IdNotFoundException()
			: ResourcesException("IdNotFoundException", std::string("Couldn't find Resource ID for value"))
		{}
	};
	struct FileAlreadyExistsException : ResourcesException
	{
		FileAlreadyExistsException(const fs::path& path)
			: ResourcesException("FileAlreadyExistsException", path.string())
		{}
	};
	struct ExternalLocationException : ResourcesException
	{
		ExternalLocationException(const fs::path& path)
			: ResourcesException("ExternalLocationException", path.string().append(" is not a project location"))
		{}
	};

	template<class T>
	class Resources
	{
	public:
		using ResourceType = T;

		T& Get(const fs::path&);
		void Add(const T&, const fs::path&);
		void SaveAs(const T&, const fs::path&);
		void Save(const T&) const;
		void Reload(T&&);

		fs::path AbsolutePath(const T&) const;
		fs::path Uri(const T&) const;
		bool Exists(const fs::path&);

		static Resources& Instance();
	private:
		T Load(const fs::path&) const;
		void Set(T&, const T&) const;
		void Save(const T&, const fs::path&) const;

		fs::path ImportFile(const fs::path&) const;
		bool IsExternal(const fs::path&) const;
		fs::path Relative(const fs::path&) const;
		fs::path Absolute(const fs::path&) const;
		void ParentPath(const fs::path& path) { parentPath = path; }
		Resources();
	private:
		std::unordered_map<fs::path, T> cache;
		fs::path parentPath;
	};

	using ModelResources = Resources<std::shared_ptr<Components::Model>>;
	using MeshResources = Resources<Renderer::Mesh>;
	using TextureResources = Resources<std::shared_ptr<Renderer::Texture2D>>;

	enum class ModelTemplate { Cube, Sphere };
	class ResourceUtils
	{
	public:
		static ModelResources::ResourceType& ImportModel(const fs::path&);
		static ModelResources::ResourceType& GetTemplate(ModelTemplate);
		static void ExportModel(const ModelResources::ResourceType&, const fs::path&);
		static std::string GetFileName(fs::path);
	};
}