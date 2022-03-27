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

	enum class BufferAccess
	{
		None = 0,
		Read = GL_MAP_READ_BIT, Write = GL_MAP_WRITE_BIT, ReadWrite = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT
	};

	enum BufferStorageFlags
	{
		None = 0,
		Coherent = GL_MAP_COHERENT_BIT,
		ClientStorage = GL_CLIENT_STORAGE_BIT,
		ReadAccess = BufferAccess::Read, WriteAccess = BufferAccess::Write,
		Persistent = GL_MAP_PERSISTENT_BIT
	};

	inline BufferStorageFlags operator|(const BufferStorageFlags& l, const BufferStorageFlags& r)
	{
		return static_cast<BufferStorageFlags>(static_cast<GLenum>(l) | static_cast<GLenum>(r));
	}
	inline BufferStorageFlags operator&(const BufferStorageFlags& l, const BufferStorageFlags& r)
	{
		return static_cast<BufferStorageFlags>(static_cast<GLenum>(l) & static_cast<GLenum>(r));
	}
	inline BufferAccess operator|(const BufferAccess& l, const BufferAccess& r)
	{
		return static_cast<BufferAccess>(static_cast<GLenum>(l) | static_cast<GLenum>(r));
	}
	inline BufferAccess operator&(const BufferAccess& l, const BufferAccess& r)
	{
		return static_cast<BufferAccess>(static_cast<GLenum>(l) & static_cast<GLenum>(r));
	}

	class Buffer : public GLObject
	{
	public:
		Buffer(GLuint handle, size_t size, BufferTarget target, BufferStorageFlags flags)
			: GLObject(handle), size(size), target(target), flags(flags)
		{}
		~Buffer();

		Buffer& Bind(BufferTarget target);
		Buffer& Bind();
		Buffer& BindBase(GLuint index);
		Buffer& BindBase();

		void CopyData(void* data, size_t size, size_t offset = 0);

		void* Map(BufferAccess);
		void Unmap();
		void* GetPersistentPtr() { return persistent; }
		template<class T>
		std::span<T> GetPersistent() { return std::span<T>((T*)persistent, size / sizeof(T)); }
		bool IsMapped() { return persistent != nullptr; }

		size_t GetSize() { return size; }
		BufferTarget GetTarget() { return target; }
		BufferStorageFlags GetFlags() { return flags; }
		static const std::unordered_map<BufferTarget, GLuint>& GetBoundBuffers();
		static void ResetBoundBufferState();

	protected:
		size_t size;
		void* persistent = nullptr;
		BufferAccess access = BufferAccess::None;

		BufferTarget target;
		GLuint base = std::numeric_limits<GLuint>::max();
		const BufferStorageFlags flags;
	private:
		GLenum GetIdentifier() override
		{
			return GL_BUFFER;
		}
	};

	class MutableBuffer : public Buffer
	{
	public:
		MutableBuffer(GLuint handle, size_t size, BufferUsage usage, BufferTarget target)
			: Buffer(handle, size, target, BufferStorageFlags::None), usage(usage)
		{}
		void Allocate(size_t size, const void* data, BufferUsage);
		void Allocate(size_t size, const void* data = nullptr);
		void Resize(size_t size);
		void SetData(const void* data, size_t size, size_t offset);

		MutableBuffer& Bind(BufferTarget target) { Buffer::Bind(target); return *this; }
		MutableBuffer& Bind() { Buffer::Bind(); return *this; }
		MutableBuffer& BindBase(GLuint index) { Buffer::BindBase(index); return *this; }
		MutableBuffer& BindBase() { Buffer::BindBase(); return *this; }
	private:
		BufferUsage usage;
	};
}