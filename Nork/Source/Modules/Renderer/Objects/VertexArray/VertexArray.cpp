#include "pch.h"
#include "VertexArray.h"

namespace Nork::Renderer {
	struct DrawElementsIndirectCommand
	{
		DrawElementsIndirectCommand(uint32_t count, uint32_t instanceCount, uint32_t firstIndex, int baseVertex)
			: count(count), instanceCount(instanceCount), firstIndex(firstIndex), baseVertex(baseVertex)
		{}
		uint32_t count; // IBO size (normally)
		uint32_t instanceCount; // instances
		uint32_t firstIndex; // IBO offset
		int baseVertex; // VBO offset
		uint32_t baseInstance = 0;
	};

	void VertexArray::MultiDrawInstanced(uint32_t count, DrawMode mode)
	{
		ibo->Bind();
		static std::vector<DrawElementsIndirectCommand> indirect;
		indirect.clear();
		indirect.push_back(DrawElementsIndirectCommand(ibo->GetSize() / sizeof(GLuint), count, 0, 0));
		glMultiDrawElementsIndirect(std::to_underlying(mode), GL_UNSIGNED_INT, indirect.data(), indirect.size(), 0);
	}
}