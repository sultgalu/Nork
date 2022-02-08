#include "pch.h"
#include "Buffer.h"
namespace Nork::Renderer2 {
	static std::unordered_map<GLenum, GLuint> boundBuffers;

	Buffer& Buffer::Bind(BufferTarget target)
	{
		this->target = static_cast<GLenum>(target);
		glBindBuffer(this->target, handle);
		boundBuffers[this->target] = handle;
	}

	const std::unordered_map<GLenum, GLuint>& Buffer::GetBoundBuffers()
	{
		return boundBuffers;
	}
}