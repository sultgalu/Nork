export module Nork.Renderer:GLTFBuilder;

export import :Material;
export import :Mesh;
export import :GLTF;

export namespace Nork::Renderer {
	class GLTFBuilder
	{
	public:
		GLTFBuilder(const std::filesystem::path& buffersPath)
			:buffersPath(buffersPath)
		{}
		GLTFBuilder& AddScene(const std::vector<int>& nodes, bool defaultScene = false);
		GLTFBuilder& AddNode(int meshIdx);
		GLTFBuilder& AddMesh(std::shared_ptr<Mesh> mesh, const std::string& name, int matIdx = -1);
		GLTFBuilder& AddMaterial(std::shared_ptr<Material> material, const std::string& name, std::vector<std::pair<TextureMap, std::string>> imageUris);
		GLTF::GLTF Get() { return gltf; }
	private:
		GLTF::GLTF gltf;
		std::filesystem::path buffersPath;
	};
}