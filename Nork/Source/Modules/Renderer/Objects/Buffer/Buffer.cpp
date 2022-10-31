#include "pch.h"
#include "Buffer.h"
namespace Nork::Renderer {
	static std::unordered_map<BufferTarget, GLuint> boundBuffers;

	Buffer::~Buffer()
	{
		Logger::Info("Deleting buffer ", handle);
		glDeleteBuffers(1, &handle);
	}
	Buffer& Buffer::BindBase(GLuint index)
	{
		glBindBufferBase(static_cast<GLenum>(target), index, handle);
		base = index;
		return *this;
	}
	Buffer& Buffer::BindBase()
	{
		BindBase(base);
		return *this;
	}
	Buffer& Buffer::Bind()
	{
		glBindBuffer(static_cast<GLenum>(target), handle);
		boundBuffers[this->target] = handle;
		return *this;
	}
	Buffer& Buffer::Bind(BufferTarget target)
	{
		this->target = target;
		return Bind();
	}

	void Buffer::SubData(void* data, size_t size, size_t offset)
	{
		if (flags & BufferStorageFlags::DynamicStorage)
		{
			glBufferSubData(static_cast<GLenum>(target), offset, size, data);
		}
		else
		{
			Logger::Error("Can't call glBufferSubData, DynamicStorage flag not set");
		}
	}

	void Buffer::CopyData(void* data, size_t size, size_t offset)
	{
		glGetBufferSubData(static_cast<GLenum>(target), offset, size, data);
	}
	void Buffer::CopyDataFrom(Buffer& other)
	{
		size_t copySize = std::min(other.GetSize(), size);

		glBindBuffer(GL_COPY_READ_BUFFER, other.GetHandle());
		glBindBuffer(GL_COPY_WRITE_BUFFER, GetHandle());
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, copySize);
	}
	void* Buffer::Map(BufferAccess access)
	{
		if ((access & static_cast<BufferAccess>(this->flags)) != access)
		{ // storage was created with more restrictive access levels
			std::abort();
		}
		this->access = access;
		auto mapFlags = this->flags & ~(BufferStorageFlags::ReadAccess | BufferStorageFlags::WriteAccess);
		mapFlags |= static_cast<BufferStorageFlags>(access);
		persistent = glMapBufferRange(static_cast<GLenum>(target), 0, size, mapFlags);
		return persistent;
	}
	void Buffer::Unmap()
	{
		glUnmapBuffer(static_cast<GLenum>(target));
		persistent = nullptr;
		this->access = BufferAccess::None;
	}
	
	const std::unordered_map<BufferTarget, GLuint>& Buffer::GetBoundBuffers()
	{
		return boundBuffers;
	}
	void Buffer::ResetBoundBufferState()
	{
		boundBuffers.clear();
	}

	void MutableBuffer::Allocate(size_t capacitiy, const void* data, BufferUsage usage)
	{
		// Logger::Debug("alloc");
		glBufferData(static_cast<GLenum>(target), capacitiy, data, static_cast<GLenum>(usage));
		this->usage = usage;
		this->size = capacitiy;
	}
	void MutableBuffer::Allocate(size_t capacitiy, const void* data)
	{
		Allocate(capacitiy, data, usage);
	}
	void MutableBuffer::Resize(size_t newSize)
	{
		Logger::Debug("resize");
		if (size == 0)
		{ // no data to copy
			glBufferData(static_cast<GLenum>(target), newSize, nullptr, static_cast<GLenum>(usage));
		}
		else
		{ // there is data to be copied over to new buffer
			size_t copySize = std::min(newSize, size);
			GLuint tempHandle;
			glGenBuffers(1, &tempHandle);
			glBindBuffer(GL_COPY_WRITE_BUFFER, tempHandle);
			glBufferData(GL_COPY_WRITE_BUFFER, copySize, nullptr, static_cast<GLenum>(BufferUsage::StreamCopy));

			glBindBuffer(GL_COPY_READ_BUFFER, handle);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, copySize);

			glBufferData(GL_COPY_READ_BUFFER, newSize, nullptr, static_cast<GLenum>(usage));
			glCopyBufferSubData(GL_COPY_WRITE_BUFFER, GL_COPY_READ_BUFFER, 0, 0, copySize);

			glDeleteBuffers(1, &tempHandle);
		}
		size = newSize;
	}
	void MutableBuffer::SetData(const void* data, size_t size, size_t offset)
	{
		if (offset + size > this->size)
		{
			Resize(offset + size);
		}
		glBufferSubData(static_cast<GLenum>(target), offset, size, data);
	}
}