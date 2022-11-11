#pragma once

#include "Data/Vertex.h"

namespace Nork::Renderer {
	class MeshFactory
	{
	public:
		static std::vector<Data::Vertex> CubeVertices();
		static std::vector<unsigned int> CubeIndices();
	};
}