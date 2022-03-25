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

	void Buffer::Allocate(size_t capacitiy, const void* data)
	{
		Logger::Debug("alloc");
		glBufferData(static_cast<GLenum>(target), capacitiy, data, static_cast<GLenum>(usage));
		this->size = data == nullptr ? 0 : capacitiy;
		this->capacity = capacitiy;
	}
	void Buffer::ShrinkToFit()
	{
		if (capacity > size)
		{
			ResizeBuffer(size);
		}
	}
	void Buffer::Reserve(size_t cap)
	{
		if (this->capacity < cap)
		{
			ResizeBuffer(cap);
		}
	}
	void Buffer::Resize(size_t newSize)
	{
		if (newSize <= capacity)
		{
			size = newSize;
		}
		else
		{
			ResizeBuffer(newSize);
		}
	}
	void Buffer::SetData(const void* data, size_t size, size_t offset)
	{
		if (offset + size > this->capacity)
		{
			ResizeBuffer(offset + size);
		}
		glBufferSubData(static_cast<GLenum>(target), offset, size, data);
		if (offset + size > this->size)
		{
			this->size = offset + size;
		}
	}
	void Buffer::Append(const void* data, size_t size)
	{
		if (this->capacity == 0)
		{ // glBufferData(data) instead of glBufferData(0)->glBufferSubData(data)
			Allocate(size, data);
		}
		else
		{
			SetData(data, size, this->size);
		}
	}
	void Buffer::ResizeBuffer(size_t newSize)
	{
		Logger::Debug("resize");
		if (size == 0)
		{ // no data to copy
			glBufferData(static_cast<GLenum>(target), newSize, nullptr, static_cast<GLenum>(usage));
		}
		else
		{ // there is data to be copied over to new buffer
			GLuint tempHandle;
			glGenBuffers(1, &tempHandle);
			glBindBuffer(GL_COPY_WRITE_BUFFER, tempHandle);
			glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, static_cast<GLenum>(BufferUsage::StreamCopy));

			glBindBuffer(GL_COPY_READ_BUFFER, handle);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

			glBufferData(GL_COPY_READ_BUFFER, newSize, nullptr, static_cast<GLenum>(usage));
			glCopyBufferSubData(GL_COPY_WRITE_BUFFER, GL_COPY_READ_BUFFER, 0, 0, size);

			glDeleteBuffers(1, &tempHandle);
		}
		capacity = newSize;
	}
	void Buffer::GetData(void* data, size_t size, size_t offset)
	{
		glGetBufferSubData(static_cast<GLenum>(target), offset, size, data);
	}
	void Buffer::Erase(size_t offset, size_t length)
	{
		Logger::Debug("erase");
		char* data = (char*)glMapBufferRange(static_cast<GLenum>(target), offset, size - offset, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
		std::memmove(data, data + length, size - (offset + length));
		glUnmapBuffer(static_cast<GLenum>(target));
		size -= length;
	}
	void Buffer::Erase(std::vector<std::pair<size_t, size_t>> ranges)
	{
		Logger::Debug("erase");
		using _Range = std::pair<size_t, size_t>;
		std::sort(ranges.begin(), ranges.end(), [](const _Range& r1, const _Range& r2)
			{ // decreasing order
				return r1 > r2;
			});

		size_t firstOffset = ranges.back().first;
		char* data = (char*)glMapBufferRange(static_cast<GLenum>(target), firstOffset, size - firstOffset, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
		for (auto& range : ranges)
		{
			auto dst = data + (range.first - firstOffset);
			std::memmove(dst, dst + range.second, size - (range.first + range.second));
			size -= range.second;
		}
		glUnmapBuffer(static_cast<GLenum>(target));
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