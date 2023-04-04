#pragma once
#include "GLTF.h"
#include "../Model/Mesh.h"
#include "../Model/Material.h"

namespace Nork::Renderer {
	class GLTFBuilder
	{
	public:
		GLTFBuilder& AddScene(bool defaultScene = false);
		GLTFBuilder& AddNode(int meshIdx); // adds it to current scene
		GLTFBuilder& AddTransform(const glm::mat4&); // adds it to current node
		GLTFBuilder& AddMesh(const std::string& name = "");
		GLTFBuilder& AddPrimitive(const Mesh& mesh, const std::filesystem::path& buffersPath, int matIdx = -1); // adds it to current mesh
		GLTFBuilder& AddMaterial(const Material& material, std::vector<std::pair<TextureMap, std::string>> imageUris, const std::string& name = "");
		GLTF::GLTF Get() { return gltf; }
	private:
		GLTF::GLTF gltf;
		std::unordered_map<std::string, uint32_t> texturesByImageUri;
	};
}