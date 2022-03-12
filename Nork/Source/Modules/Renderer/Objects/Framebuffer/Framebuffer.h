#pragma once
#include "../Texture/Texture.h"

namespace Nork::Renderer {
	struct FramebufferAttachments
	{
		std::shared_ptr<Texture> depth = nullptr;
		std::vector<std::pair<std::shared_ptr<Texture>, int>> colors;
		FramebufferAttachments& Color(std::shared_ptr<Texture> texture, int idx)
		{
			colors.push_back({ texture, idx });
			return *this;
		}
		FramebufferAttachments& Depth(std::shared_ptr<Texture> texture)
		{
			depth = texture;
			return *this;
		}
	};

	class Framebuffer: public GLObject
	{
	public:
		Framebuffer(GLuint handle, int width, int height, FramebufferAttachments attachments)
			: GLObject(handle), width(width), height(height), attachments(attachments), clearBits(GetClearBits(attachments))
		{}
		~Framebuffer()
		{
			Logger::Info("Deleting framebuffer ", handle, ".");
			glDeleteFramebuffers(1, &handle);
		}
		static void BindDefault()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		Framebuffer& Bind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, handle);
			return *this;
		}
		Framebuffer& SetViewport()
		{
			glViewport(0, 0, width, height);
			return *this;
		}
		Framebuffer& Clear()
		{
			glClear(clearBits);
			return *this;
		}
		const FramebufferAttachments& GetAttachments() { return attachments; }
	protected:
		static GLenum GetClearBits(FramebufferAttachments& attachments)
		{
			GLenum clearBits = 0;
			if (attachments.depth != nullptr)
				clearBits |= GL_DEPTH_BUFFER_BIT;
			if (!attachments.colors.empty())
				clearBits |= GL_COLOR_BUFFER_BIT;
			return clearBits;
		}
	protected:
		const int width, height;
		GLenum clearBits;
		const FramebufferAttachments attachments;
	};

}

