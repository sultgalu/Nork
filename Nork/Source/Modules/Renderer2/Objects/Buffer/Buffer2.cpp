#include "pch.h"
#include "Buffer.h"
namespace Nork::Renderer {
	static std::unordered_map<GLenum, GLuint> boundBuffers;

	Buffer& Buffer::Bind(BufferTarget target)
	{
		this->target = static_cast<GLenum>(target);
		glBindBuffer(this->target, handle);
		boundBuffers[this->target] = handle;
		return *this;
	}

	const std::unordered_map<GLenum, GLuint>& Buffer::GetBoundBuffers()
	{
		return boundBuffers;
	}
}