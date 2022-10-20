export module Nork.Core:ResourceManager;

import Nork.Components;
import Nork.Renderer;

export namespace Nork {
	struct MeshResource
	{
		std::string mesh;
		std::string material;
	};
	struct ModelResource
	{
		std::vector<MeshResource> meshes;
	};

	class ResourceManager
	{
	public:
		ResourceManager(Renderer::DrawState& drawState);
		std::string ImportTexture(const std::string& path);
		std::shared_ptr<Components::Model> ImportModel(const std::string& path);
		void ExportModel(std::shared_ptr<Components::Model>);

		std::shared_ptr<Components::Model> GetModelByPath(const std::string& path);
		std::shared_ptr<Components::Model> GetModel(const std::string& id);
		std::shared_ptr<Renderer::Mesh> GetMesh(const std::string& id);
		std::shared_ptr<Renderer::Texture2D> GetTextureByPath(const std::string& path);
		std::shared_ptr<Renderer::Texture2D> GetTexture(const std::string& id);
		std::shared_ptr<Renderer::Material> GetMaterialByPath(const std::string& path);
		std::shared_ptr<Renderer::Material> GetMaterial(const std::string& id);

		Renderer::DrawState& drawState;

		std::optional<std::string> PathFor(std::shared_ptr<Renderer::Texture2D>);
		std::optional<std::string> PathFor(std::shared_ptr<Renderer::Material>);
		std::optional<std::string> PathFor(std::shared_ptr<Renderer::Mesh>);
		std::optional<std::string> PathFor(std::shared_ptr<Components::Model>);
		std::optional<std::string> IdFor(std::shared_ptr<Renderer::Texture2D>);
		std::optional<std::string> IdFor(std::shared_ptr<Renderer::Material>);
		std::optional<std::string> IdFor(std::shared_ptr<Renderer::Mesh>);
		std::optional<std::string> IdFor(std::shared_ptr<Components::Model>);

		std::filesystem::path AssetsPath();
		std::filesystem::path ModelsPath();
		std::filesystem::path MeshesPath();
		std::filesystem::path BufferBinariesPath();
		std::filesystem::path MaterialsPath();
		std::filesystem::path ImagesPath();
		std::filesystem::path ExportPath();

		std::shared_ptr<Components::Model> CloneModel(std::shared_ptr<Components::Model>, const std::string& id);
		void SaveModel(std::shared_ptr<Components::Model>);
		std::shared_ptr<Renderer::Material> CloneMaterial(std::shared_ptr<Renderer::Material>, const std::string& id);
		void SaveMaterial(std::shared_ptr<Renderer::Material>);
	private:
		std::unordered_map<std::string, std::shared_ptr<Components::Model>> models;
		std::unordered_map<std::string, std::shared_ptr<Renderer::Texture2D>> textures;
		std::unordered_map<std::string, std::shared_ptr<Renderer::Mesh>> meshes;
		std::unordered_map<std::string, std::shared_ptr<Renderer::Material>> materials;

		void LoadModel(const std::string& id);
		void LoadTexture(const std::string& id);
		void LoadMesh(const std::string& id);
		void LoadMaterial(const std::string& id);

		std::string ExternalPathToAssetId(const std::string&);
		std::filesystem::path AssetIdToTexturePath(const std::string&);
		std::filesystem::path AssetIdToMeshPath(const std::string&);
		std::filesystem::path AssetIdToMaterialPath(const std::string&);
		std::filesystem::path AssetIdToModelPath(const std::string&);
		std::filesystem::path assetsPath;

		void SaveMesh(std::shared_ptr<Renderer::Mesh>);
		void SaveImage(std::shared_ptr<Renderer::Texture2D>, const std::string& path);
		void SaveGLTF(const Renderer::GLTF::GLTF& gltf, const std::string& path);
		Renderer::GLTF::GLTF LoadGLTF(const std::string& path);
	};
}