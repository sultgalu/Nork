#pragma once

#include "../GLObject.h"

namespace Nork::Renderer {
	enum class BufferTarget : GLenum
	{
		Vertex = GL_ARRAY_BUFFER, Index = GL_ELEMENT_ARRAY_BUFFER,
		VertexArray = GL_VERTEX_ARRAY,

		SSBO = GL_SHADER_STORAGE_BUFFER, UBO = GL_UNIFORM_BUFFER,
		AtomicCounter = GL_ATOMIC_COUNTER_BUFFER
	};

	enum class BufferUsage
	{
		DynamicDraw = GL_DYNAMIC_DRAW, StaticDraw = GL_STATIC_DRAW
	};

	class Buffer: public GLObject
	{
	public:
		Buffer& Create()
		{
			Logger::Info("Creating buffer ", handle, ".");
			glGenBuffers(1, &handle);
			return *this;
		}
		void Destroy()
		{
			Logger::Info("Deleting buffer ", handle, ".");
			glDeleteBuffers(1, &handle);
		}
		Buffer& Bind(BufferTarget target);
		inline void SetData(const void* data, size_t size, size_t offset = 0)
		{
			if (this->size < size + offset)
			{
				Logger::Error("Not enough memory in buffer(", this->size, "). for offset " + offset, " and size ", size);
				return;
			}
			glBufferSubData(target, offset, size, data);
		}
		inline void Resize(size_t size)
		{
			if (this->size != size)
			{
				Allocate(size);
			}
		}
		inline void Allocate(size_t size, const void* data = nullptr)
		{
			glBufferData(target, size, data, usage);
			this->size = size;
		}
		inline void GetData(void* data, size_t size, size_t offset = 0)
		{
			glGetBufferSubData(target, offset, size, data);
		}
		Buffer& BindBase(GLuint index)
		{
			glBindBufferBase(target, index, handle);
			return *this;
		}
		inline size_t Size() { return size; }
		const std::unordered_map<GLenum, GLuint>& GetBoundBuffers();
	private:
		size_t size = 0;

		GLenum usage = GL_STATIC_DRAW;
		GLenum target;
	};
}

