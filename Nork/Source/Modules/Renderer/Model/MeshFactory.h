#pragma once

#include "Mesh.h"

namespace Nork::Renderer {
	class MeshFactory
	{
	public:
		MeshFactory(VAO& vaoWrapper)
			: vaoWrapper(vaoWrapper)
		{}
		std::shared_ptr<Mesh> Create(const std::vector<Data::Vertex>& vertices, const std::vector<uint32_t>& indices);
		std::shared_ptr<Mesh> CreateCube();
	private:
		VAO& vaoWrapper;
	};
}