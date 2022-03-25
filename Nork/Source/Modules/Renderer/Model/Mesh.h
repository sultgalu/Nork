#pragma once

namespace Nork::Renderer {
	namespace Model {
		struct Vertex
		{
			glm::vec3 position, normal;
			glm::vec2 texCoords;
			glm::vec3 tangent;
		};
	}
	class Mesh
	{
	public:
		Mesh(uint32_t idx)
			: storageIdx(idx)
		{}
		uint32_t storageIdx;
	};
	// struct SubMesh
	// {
	// 	uint32_t vertOffs, vertCount;
	// 	uint32_t indexOffs, indexCount;
	// 	int materialIdx;
	// };

	// class Mesh
	// {
	// public:
	// 	Mesh(size_t initialVboSize = 0, size_t initialIboSize = 0);
	// 	void Draw() const;
	// 	void DrawInstanced(uint32_t count) const;
	// 	static Mesh Cube();
	// 
	// 	SubMesh& AddSubMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	// 
	// 	std::shared_ptr<VertexArray> vao;
	// 	std::vector<SubMesh> subMeshes;
	// private:
	// };	
}