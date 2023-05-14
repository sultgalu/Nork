#pragma once
#include "../Model/Mesh.h"
#include "gltf.h"

namespace Nork::Renderer {
	class GLTFReader
	{
	public:
		GLTFReader(const fs::path& path);
		std::shared_ptr<Model> Read();
		// gltf files that contain 1 exported model usually have 1 node (mesh). But if it has more, this function recursively processes them and loads everything as 1 model
		// eg. if gltf contains a complex scene, the while scene will be loaded as a singular model
		void AddNodeRecursive(int nodeIdx, const glm::mat4& parentTransform = glm::identity<glm::mat4>());
		std::shared_ptr<Material> CreateMaterial(const GLTF::Material& mat);
		std::shared_ptr<MeshData> CreateRendererMesh(int idx, int meshIdx);
		template<class T> std::vector<uint32_t> GetIndices(const GLTF::Primitive& prim);
		void SetTangent(Data::Vertex& v1, Data::Vertex& v2, Data::Vertex& v3);
		template<class S, class T = float> std::span<T> BufferView(const GLTF::Accessor& accessor);
		fs::path AbsolutePath(std::string uri) { return srcFolder / uri; }
	public:
		GLTF::GLTF gltf;
	private:
		fs::path srcFolder; // absolute
		fs::path dstFolder; // relative

		std::vector<std::vector<char>> buffers;
		std::vector<std::shared_ptr<Texture>> images;
		std::vector<std::shared_ptr<Material>> materials;
		std::vector<std::vector<std::shared_ptr<MeshData>>> meshDatas;
		std::shared_ptr<Material> defaultMaterial;

		std::shared_ptr<Model> model;
	};
}