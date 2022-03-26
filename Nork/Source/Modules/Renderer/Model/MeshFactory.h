#pragma once

#include "Mesh.h"

namespace Nork::Renderer {
	class MeshFactory
	{
	public:
		MeshFactory(VertexArrayWrapper& vaoWrapper)
			: vaoWrapper(vaoWrapper)
		{
			Logger::Info("Mesh factory CREATED", "-----------------------------------");
		}
		std::shared_ptr<Mesh> Create(const std::vector<Data::Vertex>& vertices, const std::vector<uint32_t>& indices);
		std::shared_ptr<Mesh> CreateCube();
	private:
		VertexArrayWrapper& vaoWrapper;
	};
}