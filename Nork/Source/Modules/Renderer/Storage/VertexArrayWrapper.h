#pragma once

#include "TypedBufferWrapper.h"
#include "../Objects/VertexArray/VertexArray.h"
#include "../Data/Vertex.h"

namespace Nork::Renderer {
	using VertexBufferWrapper = TypedBufferWrapper<Data::Vertex, BufferTarget::Index>;
	using IndexBufferWrapper = TypedBufferWrapper<uint32_t, BufferTarget::Index>;

	class VertexArrayWrapper
	{
	public:
		VertexArrayWrapper(); 
		VertexBufferWrapper& GetVertexWrapper() { return vertexBufferWrapper; }
		IndexBufferWrapper& GetIndexWrapper() { return indexBufferWrapper; }
		std::shared_ptr<VertexArray> GetVertexArray() { return vao; }
	private:
		VertexBufferWrapper vertexBufferWrapper;
		IndexBufferWrapper indexBufferWrapper;
		std::shared_ptr<VertexArray> vao;
	};
}