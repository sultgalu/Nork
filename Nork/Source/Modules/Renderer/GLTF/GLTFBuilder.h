#pragma once
#include "GLTF.h"
#include "../Model/Mesh.h"
#include "../Model/Material.h"

namespace Nork::Renderer {
	class GLTFBuilder
	{
	public:
		GLTFBuilder& AddScene(bool defaultScene = false);
		GLTFBuilder& AddMesh(const Mesh& mesh, const std::string& name, int matIdx = -1, const std::filesystem::path& buffersPath = "");
		GLTFBuilder& AddMaterial(const Material& material, std::vector<std::pair<TextureMap, std::string>> imageUris, const std::string& name = "");
		GLTF::GLTF Get() { return gltf; }
	private:
		GLTF::GLTF gltf;
	};
}