#pragma once

#include "../Storage/VertexArrayWrapper.h"

namespace Nork::Renderer {
	class Mesh
	{
	public:
		Mesh(VertexArrayWrapper& vaoWrapper,
			std::shared_ptr<size_t> vertexIdx, std::shared_ptr<size_t> indexIdx,
			size_t vertexCount, size_t indexCount)
			: vaoWrapper(vaoWrapper),
			vertexIdx(vertexIdx), indexIdx(indexIdx),
			vertexCount(vertexCount), indexCount(indexCount)
		{}
		Mesh(Mesh&&) = delete;

		size_t GetVertexOffset() { return *vertexIdx; }
		size_t GetIndexOffset() { return *indexIdx; }
		size_t GetVertexCount() { return vertexCount; }
		size_t GetIndexCount() { return indexCount; }

		VertexArrayWrapper& GetVaoWrapper() { return vaoWrapper; }
	private:
		std::shared_ptr<size_t> vertexIdx;
		std::shared_ptr<size_t> indexIdx;
		size_t vertexCount;
		size_t indexCount;
		VertexArrayWrapper& vaoWrapper;
	};
}