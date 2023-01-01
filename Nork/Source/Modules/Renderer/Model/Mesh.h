#pragma once

#include "../Data/Vertex.h"
#include "../DeviceResource.h"

namespace Nork::Renderer {
	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(std::shared_ptr<BufferView<Data::Vertex>> vertices, std::shared_ptr<BufferView<uint32_t>> indices)
			: vertices(vertices), indices(indices)
		{}
	public:
		std::shared_ptr<BufferView<Data::Vertex>> vertices;
		std::shared_ptr<BufferView<uint32_t>> indices;
	};
}