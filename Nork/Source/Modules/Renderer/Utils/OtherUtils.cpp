#include "../Utils.h"

namespace Nork::Renderer::Utils::Other
{
	void ReadPixels(unsigned int fbo, int colorAtt, int x, int y, Texture::Format format, void* buf)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + colorAtt); // namedfbReadPixels doesn't work
		glReadPixels(x, y, 1, 1, Texture::GetFormat(format), Texture::GetType(format), buf);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void ReadPixels(GLuint tex, int x, int y, Texture::Format format, void* buf)
	{
		glGetTextureSubImage(tex, 0, x, y, 0, 1, 1, 1, Texture::GetFormat(format), Texture::GetType(format), 4, buf);
	}
	unsigned int CreateUBO(int idx, int size, GLenum usage)
	{
		unsigned int ubo;
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage);
		glBindBufferBase(GL_UNIFORM_BUFFER, idx, ubo);
		return ubo;
	}
}