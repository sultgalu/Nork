#pragma once

#include "../Utils.h"

namespace Nork::Renderer
{
	template<typename T>
	concept VertexLayoutCompatible = false
		|| std::same_as<T, float>
		|| std::same_as<T, glm::vec2>
		|| std::same_as<T, glm::vec3>
		|| std::same_as<T, glm::vec4>;

	static void SetVaoAttribs2(std::vector<int> attrLens)
	{
		int stride = 0;
		for (int i = 0; i < attrLens.size(); i++)
			stride += attrLens[i];
		stride *= sizeof(float);

		int offset = 0;
		for (int i = 0; i < attrLens.size(); i++)
		{
			glVertexAttribPointer(i, attrLens[i], GL_FLOAT, false, stride, (void*)(offset * sizeof(float)));
			glEnableVertexAttribArray(i);
			offset += attrLens[i];
		}
	}

	template<bool IB, VertexLayoutCompatible... Types>
	class VertexArray
	{
	public:
		VertexArray()
		{
			glGenBuffers(1, &vb);
			glBindBuffer(GL_ARRAY_BUFFER, vb);
			glBufferData(GL_ARRAY_BUFFER, 1000, 0, GL_DYNAMIC_DRAW);

			/*if constexpr (IB)
			{
				glGenBuffers(1, &ib);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
			}*/

			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			AddAttrib<Types...>(0, 0);
		}

		void Bind()
		{
			glBindVertexArray(vao);
		}
		void Draw(size_t count, int first = 0)
		{
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, first, count);
		}
		void DrawElements(std::span<uint32_t> indices, GLenum mode = GL_TRIANGLES)
		{
			glBindVertexArray(vao);
			glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, indices.data());
		}
		void SetVBData(size_t size, void* data, GLenum usage)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vb);
			glBufferData(GL_ARRAY_BUFFER, size, data, usage);
		}
		void SetVBSubData(size_t offs, size_t size, void* data)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vb);
			glBufferSubData(GL_ARRAY_BUFFER, offs, size, data);
		}
		/*template < typename = typename std::enable_if<IB>::type>
		void SetIBData(size_t size, void* data, GLenum usage)
		{
			glBindBuffer(GL_VERTEX_ARRAY, ib);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
		}
		template < typename = typename std::enable_if<IB>::type>
		void SetIBSubData(size_t offs, size_t size, void* data)
		{
			glBindBuffer(GL_VERTEX_ARRAY, ib);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offs, size, data);
		}*/
		static consteval size_t VertexSize()
		{
			return VertexSize<Types...>();
		}
	private:
		template<typename T, typename... Rest>
		void AddAttrib(int i, int offs)
		{
			auto size = VertexSize<Types...>();
			glVertexAttribPointer(i, sizeof(T) / sizeof(float), GL_FLOAT, false, VertexSize<Types...>(), (void*)offs);
			glEnableVertexAttribArray(i);
			if constexpr (sizeof...(Rest) > 0)
			{
				AddAttrib<Rest...>(i + 1, offs + sizeof(T));
			}
		}
		template<typename T, typename... Rest>
		static consteval size_t VertexSize()
		{
			if constexpr (sizeof...(Rest) > 0)
				return sizeof(T) + VertexSize<Rest...>();
			else return sizeof(T);
		}
	public:
		GLuint vao;
		//std::enable_if<IB, GLuint>::type ib;
		GLuint vb;
	};
}