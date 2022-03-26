#pragma once

#include "../GLObject.h"

namespace Nork::Renderer {
	enum class BufferTarget : GLenum
	{
		None = GL_NONE,

		Vertex = GL_ARRAY_BUFFER, Index = GL_ELEMENT_ARRAY_BUFFER,
		VertexArray = GL_VERTEX_ARRAY,

		SSBO = GL_SHADER_STORAGE_BUFFER, UBO = GL_UNIFORM_BUFFER,
		AtomicCounter = GL_ATOMIC_COUNTER_BUFFER,

		CopyWrite = GL_COPY_WRITE_BUFFER, CopyRead = GL_COPY_READ_BUFFER,
		TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER
	};

	enum class BufferUsage
	{
		None = GL_NONE,

		DynamicDraw = GL_DYNAMIC_DRAW, StaticDraw = GL_STATIC_DRAW, StreamDraw = GL_STREAM_DRAW,
		DynamicRead = GL_DYNAMIC_READ, StaticRead = GL_STATIC_READ, StreamRead = GL_STREAM_READ,
		DynamicCopy = GL_DYNAMIC_COPY, StaticCopy = GL_STATIC_COPY, StreamCopy = GL_STREAM_COPY,
	};

	class Buffer : public GLObject
	{
	public:
		Buffer(GLuint handle, size_t size, size_t capacity, BufferUsage usage, BufferTarget target)
			: GLObject(handle), size(size), capacity(capacity), usage(usage), target(target)
		{}
		~Buffer();

		Buffer& Bind(BufferTarget target);
		Buffer& Bind();
		Buffer& BindBase(GLuint index);
		Buffer& BindBase();

		void Allocate(size_t size, const void* data = nullptr);
		void ShrinkToFit();
		void Reserve(size_t cap);
		void Resize(size_t size);
		void SetData(const void* data, size_t size, size_t offset = 0);
		void Append(const void* data, size_t size);
		void GetData(void* data, size_t size, size_t offset = 0);
		void Erase(size_t offset, size_t length);
		void Erase(std::vector<std::pair<size_t, size_t>> ranges); // offset, length
		Buffer& Clear() { size = 0; return *this; }

		template<class T>
		size_t GetCount() { return size / sizeof(T); }
		size_t GetSize() { return size; }
		size_t GetCapacity() { return capacity; }
		BufferTarget GetTarget() { return target; }
		BufferUsage GetUsage() { return usage; }
		static const std::unordered_map<BufferTarget, GLuint>& GetBoundBuffers();
		static void ResetBoundBufferState();

		template<class T>
		void Append(const std::vector<T>& data) { Append(data.data(), data.size() * sizeof(T)); }
	protected:
		size_t size;
		size_t capacity;
		BufferTarget target;
		GLuint base = std::numeric_limits<GLuint>::max();
		const BufferUsage usage;
	private:
		void ResizeBuffer(size_t newSize);
		GLenum GetIdentifier() override
		{
			return GL_BUFFER;
		}
	};
}