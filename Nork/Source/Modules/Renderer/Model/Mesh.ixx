export module Nork.Renderer:Mesh;

export import :VertexArrayWrapper;

export namespace Nork::Renderer {
	class Mesh
	{
	public:
		Mesh(VAO& vaoWrapper,
			std::shared_ptr<Data::Vertex*> vertexIdx, std::shared_ptr<uint32_t*> indexIdx,
			size_t vertexCount, size_t indexCount)
			: vaoWrapper(vaoWrapper),
			vertexIdx(vertexIdx), indexIdx(indexIdx),
			vertexCount(vertexCount), indexCount(indexCount)
		{}
		Mesh(Mesh&&) = delete;

		std::shared_ptr<Data::Vertex*> GetVertexPtr() { return vertexIdx; }
		std::shared_ptr<uint32_t*> GetIndexPtr() { return indexIdx; }
		size_t GetVertexCount() { return vertexCount; }
		size_t GetIndexCount() { return indexCount; }

		VAO& GetVaoWrapper() { return vaoWrapper; }
	private:
		std::shared_ptr<Data::Vertex*> vertexIdx;
		std::shared_ptr<uint32_t*> indexIdx;
		size_t vertexCount;
		size_t indexCount;
		VAO& vaoWrapper;
	};
}