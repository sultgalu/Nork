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
		//template<class S>
		//requires requires(S s)
		//{
		//	sizeof(S) == SizeOf<T...>(); 
		//}
		//void UploadVertices(std::vector<S> vertices, glm::mat4 model) // vertices is a copy
		//{
		//	for (size_t i = 0; i < vertices.size(); i++)
		//	{
		//		vertices[i] = model * glm::vec4	(vertices[i], 1);
		//	}
		//	vao.SetVBData(vertices.size() * vao.VertexSize(), vertices.data(), GL_DYNAMIC_DRAW);
		//}
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
		void DrawAsTriangle(uint32_t count, uint32_t offset = 0)
		{
			vao.Draw(count, offset);
		}
		void DrawAsLines(uint32_t count, uint32_t offset = 0)
		{
			vao.Draw(count, offset, GL_LINES);
		}
		void DrawAsPoints(uint32_t count, uint32_t offset = 0)
		{
			vao.Draw(count, offset, GL_POINTS);
		}
	private:
		VertexArray<false, T...> vao = VertexArray<false, T...>();
		GLuint vao2;
		GLuint vb;
		GLuint ib;
	};
}

