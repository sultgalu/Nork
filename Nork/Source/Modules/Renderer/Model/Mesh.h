#pragma once

#include "../Storage/SmartMappedBuffer.h"
#include "../Data/Vertex.h"

namespace Nork::Renderer {
	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const SmartMappedBuffer<Data::Vertex>::Array& vertices, const SmartMappedBuffer<uint32_t>::Array indices)
			: vertices(vertices), indices(indices)
		{}
		bool operator==(const Mesh& other) const { return other.vertices == vertices && other.indices == indices; }
		SmartMappedBuffer<Data::Vertex>::Array& Vertices() { return vertices; }
		const SmartMappedBuffer<Data::Vertex>::Array& Vertices() const { return vertices; }
		SmartMappedBuffer<uint32_t>::Array& Indices() { return indices; }
		const SmartMappedBuffer<uint32_t>::Array& Indices() const { return indices; }
	private:
		SmartMappedBuffer<Data::Vertex>::Array vertices;
		SmartMappedBuffer<uint32_t>::Array indices;
	};
}