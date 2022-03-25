#include "pch.h"
#include "VertexArray.h"

namespace Nork::Renderer {

	void VertexArray::MultiDrawInstanced(DrawElementsIndirectCommand* commands, uint32_t count, DrawMode mode)
	{
		ibo->Bind();
		glMultiDrawElementsIndirect(std::to_underlying(mode), GL_UNSIGNED_INT, commands, count, 0);
	}
}