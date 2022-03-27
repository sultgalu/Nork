#pragma once

#include "TypedBufferWrapper.h"
#include "../Objects/VertexArray/VertexArray.h"
#include "../Data/Vertex.h"

namespace Nork::Renderer {
	using VertexBufferWrapper = TypedBufferWrapper<Data::Vertex, BufferTarget::Vertex>;
	using IndexBufferWrapper = TypedBufferWrapper<uint32_t, BufferTarget::Index>;

	class VertexArrayWrapper
	{
	public:
		VertexArrayWrapper(size_t vertexLimit = 1000 * 1000, size_t indexLimit = 1000 * 1000);
		VertexBufferWrapper& GetVertexWrapper() { return vertexBufferWrapper; }
		IndexBufferWrapper& GetIndexWrapper() { return indexBufferWrapper; }
		std::shared_ptr<VertexArray> GetVertexArray() { return vao; }
	private:
		VertexBufferWrapper vertexBufferWrapper;
		IndexBufferWrapper indexBufferWrapper;
		std::shared_ptr<VertexArray> vao;
	};
}