#include "pch.h"
#include "Buffer.h"
namespace Nork::Renderer {
	static std::unordered_map<BufferTarget, GLuint> boundBuffers;

	Buffer& Buffer::Create()
	{
		glGenBuffers(1, &handle);
		Logger::Info("Created buffer ", handle);
		return *this;
	}
	void Buffer::Destroy()
	{
		Logger::Info("Deleting buffer ", handle);
		glDeleteBuffers(1, &handle);
		handle = 0;
	}
	void Buffer::SetData(const void* data, size_t size, size_t offset)
	{
		if (this->size < size + offset)
		{
			MetaLogger().Error("Not enough memory in buffer(", this->size, "). for offset " + offset, " and size ", size);
			return;
		}
		glBufferSubData(static_cast<GLenum>(target), offset, size, data);
	}
	void Buffer::Allocate(size_t size, const void* data, BufferUsage usage)
	{
		this->usage = usage;
		glBufferData(static_cast<GLenum>(target), size, data, static_cast<GLenum>(usage));
		this->size = size;
	}
	void Buffer::GetData(void* data, size_t size, size_t offset)
	{
		glGetBufferSubData(static_cast<GLenum>(target), offset, size, data);
	}
	Buffer& Buffer::BindBase(GLuint index)
	{
		glBindBufferBase(static_cast<GLenum>(target), index, handle);
		return *this;
	}
	Buffer& Buffer::Bind(BufferTarget target)
	{
		this->target = target;
		glBindBuffer(static_cast<GLenum>(target), handle);
		boundBuffers[this->target] = handle;
		return *this;
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