#pragma once

#include "TypedBuffers.h"
#include "../Objects/VertexArray/VertexArray.h"
#include "../Objects/VertexArray/VertexArrayBuilder.h"

namespace Nork::Renderer {
	template<class T>
	class VAO
	{
	public:
		VAO(std::vector<int> attribs = { 3, 3, 2, 3 }, size_t vertexLimit = 1000 * 1000)
			: VAO(vertexLimit)
		{
			vao = VertexArrayBuilder()
				.VBO(vertexBufferWrapper.GetBuffer())
				.Attributes(attribs)
				.Create();
		}
		VBO<T>& GetVertexWrapper() { return vertexBufferWrapper; }
		std::shared_ptr<VertexArray> GetVertexArray() { return vao; }
	protected:
		VAO(size_t vertexLimit) : vertexBufferWrapper(vertexLimit) {}
		VBO<T> vertexBufferWrapper;
		std::shared_ptr<VertexArray> vao;
	};

	template<class T>
	class IndexedVAO : public VAO<T>
	{
	public:
		IndexedVAO(std::vector<int> attribs = {3, 3, 2, 3}, size_t vertexLimit = 1000 * 1000, size_t indexLimit = 1000 * 1000)
			: VAO<T>(vertexLimit), ibo(indexLimit)
		{
			vao = VertexArrayBuilder()
				.VBO(vertexBufferWrapper.GetBuffer())
				.IBO(ibo.GetBuffer())
				.Attributes(attribs)
				.Create();
		}
		IBO& GetIndexWrapper() { return ibo; }
	private:
		using VAO<T>::vao;
		using VAO<T>::vertexBufferWrapper;
		IBO ibo;
	};

	using DefaultVAO = IndexedVAO<Data::Vertex>;
}