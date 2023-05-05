#pragma once
#include "RenderingSystem.h"
#include "Components/Drawable.h"

namespace Nork{
	class GLTFReader
	{
	public:
		GLTFReader(const fs::path& path);
		std::shared_ptr<Renderer::Model> Read();
		// gltf files that contain 1 exported model usually have 1 node (mesh). But if it has more, this function recursively processes them and loads everything as 1 model
		// eg. if gltf contains a complex scene, the while scene will be loaded as a singular model
		void AddNodeRecursive(int nodeIdx, const glm::mat4& parentTransform = glm::identity<glm::mat4>());
		std::shared_ptr<Renderer::Material> CreateMaterial(const Renderer::GLTF::Material& mat);
		std::shared_ptr<Renderer::MeshData> CreateRendererMesh(int idx, int meshIdx);
		template<class T> std::vector<uint32_t> GetIndices(const Renderer::GLTF::Primitive& prim);
		void SetTangent(Renderer::Data::Vertex& v1, Renderer::Data::Vertex& v2, Renderer::Data::Vertex& v3);
		template<class S, class T = float> std::span<T> BufferView(const Renderer::GLTF::Accessor& accessor);
		fs::path AbsolutePath(std::string uri) { return srcFolder / uri; }
	public:
		Renderer::GLTF::GLTF gltf;
	private:
		fs::path srcFolder; // absolute
		fs::path dstFolder; // relative

		std::vector<std::vector<char>> buffers;
		std::vector<std::shared_ptr<Renderer::Texture>> images;
		std::vector<std::shared_ptr<Renderer::Material>> materials;
		std::vector<std::vector<std::shared_ptr<Renderer::MeshData>>> meshDatas;
		std::shared_ptr<Renderer::Material> defaultMaterial = RenderingSystem::Instance().NewMaterial(); // should be GLTF default

		std::shared_ptr<Renderer::Model> model;
	};
}