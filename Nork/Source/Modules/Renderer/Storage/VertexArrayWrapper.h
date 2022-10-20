#pragma once

#include "TypedBuffers.h"
import Nork.Renderer;

namespace Nork::Renderer {
	class VAO
	{
	public:
		VAO(size_t vertexLimit = 1000 * 1000, size_t indexLimit = 1000 * 1000);
		DefaultVBO& GetVertexWrapper() { return vertexBufferWrapper; }
		IBO& GetIndexWrapper() { return ibo; }
		std::shared_ptr<VertexArray> GetVertexArray() { return vao; }
	private:
		DefaultVBO vertexBufferWrapper;
		IBO ibo;
		std::shared_ptr<VertexArray> vao;
	};
}