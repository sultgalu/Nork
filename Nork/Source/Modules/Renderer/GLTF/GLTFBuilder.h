#pragma once
#include "GLTF.h"
#include "../Model/Mesh.h"
#include "../Model/Material.h"

namespace Nork::Renderer {
	class GLTFBuilder
	{
	public:
		GLTFBuilder& NewScene(bool defaultScene = false);
		GLTFBuilder& NewNode(std::string name = ""); // Adds a new node (model)
		GLTFBuilder& AddMesh(const Mesh& mesh, const std::filesystem::path& buffersPath, int matIdx = -1);
		GLTFBuilder& AddMaterial(const Material& material, std::vector<std::pair<TextureMap, std::string>> imageUris, const std::string& name = "");
		GLTF::GLTF Get() { return gltf; }
	private:
		GLTF::GLTF gltf;
		std::unordered_map<std::string, uint32_t> texturesByImageUri;
	};
}