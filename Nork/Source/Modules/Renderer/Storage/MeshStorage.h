#pragma once

#include "../Model/Mesh.h"
#include "../Objects/VertexArray/VertexArray.h"

namespace Nork::Renderer {
	class MeshStorage
	{
	public:
		struct Range
		{
			uint32_t vertexOffs = 0, vertexCount = 0;
			uint32_t indexOffs = 0, indexCount = 0;
		};
		MeshStorage(size_t initialVboSize = 0, size_t initialIboSize = 0);
		std::shared_ptr<Mesh> AddCube();
		std::vector<std::shared_ptr<Mesh>> Add(const std::vector<std::pair<const std::vector<Model::Vertex>&, const std::vector<uint32_t>&>>& verticiesIndicies);
		std::shared_ptr<Mesh> Add(const std::vector<Model::Vertex>& vertices, const std::vector<uint32_t> indices);
		void FreeObsoleteData(bool shrinkToFit = false);
	// private:
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::vector<Range> ranges;
		std::shared_ptr<VertexArray> vao;
	};

}
