#include "../Utils.h"

namespace Nork::Renderer::Utils::Framebuffer
{
	Builder::Builder(int width, int height)
		: width(width), height(height)
	{
		glGenFramebuffers(1, &this->fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
	}
	Builder::Builder(int width, int height, GLuint fbo)
		: width(width), height(height), fbo(fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
	}
	Builder& Builder::AddTexture(unsigned int handler, GLenum attachment)
	{
		glFramebufferTexture(GL_FRAMEBUFFER, attachment, handler, 0);
		return *this;
	}
	Builder& Builder::AddTexture(unsigned int* handler, Texture::Format format, GLenum attachment)
	{
		using namespace Texture;
		auto texParams = TextureParams{ .wrap = Wrap::ClampToEdge, .filter = Filter::Linear, .magLinear = false, .genMipmap = false };
		*handler = Texture::Create(this->width, this->height, format, nullptr, texParams);
		glFramebufferTexture(GL_FRAMEBUFFER, attachment, *handler, 0);

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
	unsigned int Builder::GetFramebuffer()
	{
		return this->fbo;
	}
}