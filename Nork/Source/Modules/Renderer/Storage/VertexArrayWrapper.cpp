#include "pch.h"
#include "VertexArrayWrapper.h"
#include "../Objects/VertexArray/VertexArrayBuilder.h"

namespace Nork::Renderer {
	VertexArrayWrapper::VertexArrayWrapper(size_t vertexLimit, size_t indexLimit)
		: vertexBufferWrapper(vertexLimit), indexBufferWrapper(indexLimit)
	{
		vao = VertexArrayBuilder()
			.VBO(vertexBufferWrapper.GetBuffer())
			.IBO(indexBufferWrapper.GetBuffer())
			.Attributes({ 3, 3, 2, 3 })
			.Create();
	}
}