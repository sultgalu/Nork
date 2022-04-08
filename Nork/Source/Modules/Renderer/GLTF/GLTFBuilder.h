#pragma once
#include "gltf.h"
#include "../Model/Mesh.h"
#include "../Model/Material.h"

namespace Nork::Renderer {
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