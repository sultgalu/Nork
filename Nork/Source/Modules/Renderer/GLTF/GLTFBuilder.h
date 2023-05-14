#pragma once
#include "GLTF.h"
#include "../Model/Mesh.h"
#include "../Model/Material.h"

namespace Nork::Renderer {
	class GLTFBuilder
	{
	public:
		GLTFBuilder(std::function<std::string(const std::shared_ptr<Texture>&)> textureUriProvider) : textureUriProvider(textureUriProvider) {}
		GLTFBuilder& AddScene(bool defaultScene = false);
		GLTFBuilder& AddNode(MeshNode& node, const std::filesystem::path& buffersPath); // adds it to current scene
		GLTFBuilder& AddTransform(const glm::mat4&); // adds it to current node
		GLTFBuilder& AddMesh(std::shared_ptr<Mesh>, const std::filesystem::path& buffersPath, const std::string& name = "");
		GLTFBuilder& AddPrimitive(const MeshData& mesh, const std::filesystem::path& buffersPath, int matIdx, ShadingMode shaderMode); // adds it to current mesh
		GLTFBuilder& AddMaterial(const Material& material, std::vector<std::pair<TextureMap, std::string>> imageUris, const std::string& name = "");
		GLTF::GLTF Get() { return gltf; }
	private:
		GLTF::GLTF gltf;
		std::function<std::string(const std::shared_ptr<Texture>&)> textureUriProvider;
		std::unordered_map<std::string, uint32_t> texturesByImageUri;
		std::unordered_map<std::shared_ptr<Mesh>, int> meshes;
		std::unordered_map<std::shared_ptr<Material>, int> materials;
	};
}