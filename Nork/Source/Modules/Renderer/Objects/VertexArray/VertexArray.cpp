#include "pch.h"
#include "VertexArray.h"

namespace Nork::Renderer {

	void VertexArray::MultiDrawInstanced(const DrawElementsIndirectCommand* commands, uint32_t count, DrawMode mode)
	{
		ibo->Bind();
		glMultiDrawElementsIndirect(std::to_underlying(mode), GL_UNSIGNED_INT, commands, count, 0);
	}
	void VertexArray::ChangeBuffers(std::shared_ptr<Buffer> newVbo, std::shared_ptr<Buffer> newIbo)
	{
		if (newVbo)
			vbo = newVbo;
		if (newIbo)
			ibo = newIbo;

		Bind();
		vbo->Bind(BufferTarget::Vertex);
		if (ibo)
			ibo->Bind(BufferTarget::Index);

		int offset = 0;
		for (int i = 0; i < attrLens.size(); i++)
		{
			glVertexAttribPointer(i, attrLens[i], GL_FLOAT, false, stride, (void*)(offset * sizeof(float)));
			glEnableVertexAttribArray(i);
			offset += attrLens[i];
		}
	}
}