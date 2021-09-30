#include "../Utils.h"

namespace Nork::Renderer::Utils::Framebuffer
{
	Builder::Builder(int width, int height)
		: width(width), height(height)
	{
		glGenFramebuffers(1, &this->fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
	}
	Builder& Builder::AddTexture(unsigned int handler, GLenum attachment)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, handler, 0);
		return *this;
	}
	Builder& Builder::AddTexture(unsigned int* handler, Texture::Format format, GLenum attachment)
	{
		*handler = Texture::Create2D(this->width, this->height, format);
		Texture::Bind(*handler); // do you need bind??
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, *handler, 0);

		return *this;
	}
	Builder& Builder::AddRenderbuffer(unsigned int* handler, Texture::Format format, GLenum attachment)
	{
		glGenRenderbuffers(1, handler);
		glBindRenderbuffer(GL_RENDERBUFFER, *handler);
		glRenderbufferStorage(GL_RENDERBUFFER, Texture::GetInternalFormat(format), this->width, this->height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, *handler);

		return *this;
	}
	unsigned int Builder::GetFramebuffer(bool assertComplete)
	{
		return this->fbo;
	}
}