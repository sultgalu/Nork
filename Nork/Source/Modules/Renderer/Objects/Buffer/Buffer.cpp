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
	void Buffer::Allocate(size_t size, const void* data)
	{
		glBufferData(static_cast<GLenum>(target), size, data, static_cast<GLenum>(usage));
		this->size = size;
	}
	void Buffer::Allocate(BufferUsage usage, size_t size, const void* data)
	{
		this->usage = usage;
		Allocate(size, data);
	}
	void Buffer::SetData(const void* data, size_t size, size_t offset)
	{
		glBufferSubData(static_cast<GLenum>(target), offset, size, data);
	}
	void Buffer::GetData(void* data, size_t size, size_t offset)
	{
		glGetBufferSubData(static_cast<GLenum>(target), offset, size, data);
	}
	const std::unordered_map<BufferTarget, GLuint>& Buffer::GetBoundBuffers()
	{
		return boundBuffers;
	}
	void Buffer::ResetBoundBufferState()
	{
		boundBuffers.clear();
	}
}