#pragma once

#include "../Utils.h"
#include "VertexArray.h"

namespace Nork::Renderer::Pipeline
{
	template<VertexLayoutCompatible... T>
	class StreamRenderer
	{
	public:
		StreamRenderer()
		{
		}

		void UploadVertices(std::span<std::tuple<T...>> vertices)
		{

			vao.SetVBData(vertices.size() * vao.VertexSize(), vertices.data(), GL_DYNAMIC_DRAW);
		}
		void DrawAsCube(std::span<uint32_t> indices)
		{
			vao.DrawElements(indices);
		}
		void DrawAsLines(std::span<uint32_t> indices)
		{
			vao.DrawElements(indices, GL_LINES);
		}
		void DrawAsPoints(std::span<uint32_t> indices)
		{
			vao.DrawElements(indices, GL_POINTS);
		}
	private:
		VertexArray<false, T...> vao = VertexArray<false, T...>();
		GLuint vao2;
		GLuint vb;
		GLuint ib;
	};
}

