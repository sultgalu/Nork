#pragma once

#include "../Utils.h"

namespace Nork::Renderer
{
	template<typename T>
	concept VectorType = requires ()
	{
		{ T::length() } -> std::same_as<glm::length_t>;
		{ T::value_type };
	};

	template<class>
	inline static constexpr bool isVectorType = false;
	template<VectorType T>
	inline static constexpr bool isVectorType<T> = true;
	template<class T>
	struct IsVectorType
	{
		static constexpr bool value = isVectorType<T>;
	};//: std::bool_constant<isVectorType_v<T>> {};

	template<typename T>
	concept VertexLayoutCompatible = false
		|| VectorType<T>
		|| std::floating_point<T>
		|| std::integral<T>;

	template<class T>
	requires requires() { requires sizeof (T) <= 4; }
	static consteval GLenum GLTypeSingleComponent()
	{
		if constexpr (std::is_floating_point<T>::value)
		{
			if constexpr (std::is_same<T, float>::value)
				return GL_FLOAT;
			if constexpr (std::is_same<T, double>::value)
				return GL_DOUBLE;
		}
		if constexpr (std::is_integral<T>::value)
		{
			if constexpr (std::is_signed<T>::value)
			{
				if constexpr (sizeof(T) == 4)
					return GL_INT;
				if constexpr (sizeof(T) == 2)
					return GL_SHORT;
				if constexpr (sizeof(T) == 1)
					return GL_BYTE;
			}
			else
			{
				if constexpr (sizeof(T) == 4)
					return GL_UNSIGNED_INT;
				if constexpr (sizeof(T) == 2)
					return GL_UNSIGNED_SHORT;
				if constexpr (sizeof(T) == 1)
					return GL_UNSIGNED_BYTE;
			}
		}
	}

	template<VertexLayoutCompatible T>
	static consteval GLenum GLType()
	{
		if constexpr (IsVectorType<T>::value)
		{
			return GLTypeSingleComponent<T::value_type>();
		}
		else
		{
			return GLTypeSingleComponent<T>();
		}
	}

	template<VertexLayoutCompatible T>
	static consteval GLenum ComponentCount()
	{
		if constexpr (IsVectorType<T>::value)
			return T::length();
		else
			return 1;
	}

	template<VertexLayoutCompatible T>
	inline static constexpr GLenum glType;
	template<>
	inline static constexpr GLenum glType<float> = GL_FLOAT;

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
			auto type = GLType<T>();
			auto count = ComponentCount<T>();
			if constexpr (std::is_integral<T>::value)
				glVertexAttribIPointer(i, count, type, size, (void*)offs);
			else if constexpr (std::is_same<T, double>::value)
				glVertexAttribLPointer(i, count, type, size, (void*)offs);
			else glVertexAttribPointer(i, count, type, false, VertexSize<Types...>(), (void*)offs);
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