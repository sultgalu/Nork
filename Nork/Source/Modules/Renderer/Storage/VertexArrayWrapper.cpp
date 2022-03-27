#include "pch.h"
#include "VertexArrayWrapper.h"
#include "../Objects/VertexArray/VertexArrayBuilder.h"

namespace Nork::Renderer {
	VAO::VAO(size_t vertexLimit, size_t indexLimit)
		: vertexBufferWrapper(vertexLimit), ibo(indexLimit)
	{
		vao = VertexArrayBuilder()
			.VBO(vertexBufferWrapper.GetBuffer())
			.IBO(ibo.GetBuffer())
			.Attributes({ 3, 3, 2, 3 })
			.Create();
	}
}
