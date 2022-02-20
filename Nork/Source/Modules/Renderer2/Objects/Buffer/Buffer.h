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

	class Buffer: public GLObject
	{
	public:
		Buffer& Create();
		void Destroy();
		Buffer& Bind(BufferTarget target);
		void SetData(const void* data, size_t size, size_t offset = 0);
		void Allocate(size_t size, const void* data = nullptr, BufferUsage usage = BufferUsage::StaticDraw);
		Buffer& BindBase(GLuint index);
		void GetData(void* data, size_t size, size_t offset = 0);
		size_t GetSize() { return size; }
		BufferTarget GetTarget() { return target; }
		BufferUsage GetUsage() { return usage; }
		static const std::unordered_map<BufferTarget, GLuint>& GetBoundBuffers();
		static void ResetBoundBufferState();
	private:
		size_t size = 0;
		BufferUsage usage = BufferUsage::None;
		BufferTarget target = BufferTarget::None;
	};
}

