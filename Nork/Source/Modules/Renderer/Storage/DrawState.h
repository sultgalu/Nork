#pragma once

#include "VertexArrayWrapper.h"
#include "TypedBufferWrapper.h"

namespace Nork::Renderer {
	using MatrixBufferWrapper = TypedBufferWrapper<glm::mat4, BufferTarget::UBO>;

	struct DrawState
	{
	public:
		DrawState()
		{
			modelMatrixBuffer.GetBuffer()->BindBase(5);
			materialBuffer.GetBuffer()->BindBase(6);
		}
		MatrixBufferWrapper modelMatrixBuffer = MatrixBufferWrapper(std::pow(15, 3) * 2);
		MaterialBufferWrapper materialBuffer = MaterialBufferWrapper(1000);
		VertexArrayWrapper vaoWrapper = VertexArrayWrapper(1000*1000, 1000*1000);
	private:

	};
}