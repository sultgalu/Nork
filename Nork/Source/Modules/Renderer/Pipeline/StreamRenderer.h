#pragma once

#include "../Utils.h"
#include "VertexArray.h"

namespace Nork::Renderer::Pipeline
{
	template<class T, class... Rest>
	static consteval size_t SizeOf()
	{
		if constexpr (sizeof...(Rest) > 0)
			return sizeof(T) + SizeOf<Rest...>();
		else return sizeof(T);
	}

	template<VertexLayoutCompatible... T>
	class StreamRenderer
	{
	public:
		StreamRenderer()
		{
		}

		template<class S>
		requires requires() { requires sizeof(S) == SizeOf<T...>(); }
		void UploadVertices(std::span<S> vertices)
		{
			vao.SetVBData(vertices.size() * vao.VertexSize(), vertices.data(), GL_DYNAMIC_DRAW);
		}
		void DrawAsTriangle(std::span<uint32_t> indices)
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
		void DrawAsTriangle(uint32_t count)
		{
			vao.Draw(count, 0);
		}
		void DrawAsLines(uint32_t count)
		{
			vao.Draw(count, 0, GL_LINES);
		}
		void DrawAsPoints(uint32_t count)
		{
			vao.Draw(count, 0, GL_POINTS);
		}
	private:
		VertexArray<false, T...> vao = VertexArray<false, T...>();
		GLuint vao2;
		GLuint vb;
		GLuint ib;
	};
}

