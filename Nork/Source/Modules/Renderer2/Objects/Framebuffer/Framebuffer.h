#pragma once
#include "../Texture/Texture.h"

namespace Nork::Renderer2 {
	struct FramebufferAttachments
	{
		std::optional<Texture> depth;
		std::vector<std::pair<Texture, int>> colors;
		FramebufferAttachments Color(Texture texture, int idx)
		{
			colors.push_back({ texture, idx });
			return *this;
		}
		FramebufferAttachments Depth(Texture texture)
		{
			depth = texture;
			return *this;
		}
	};

	class Framebuffer: public GLObject
	{
	public:
		Framebuffer& Create()
		{
			glGenFramebuffers(1, &handle);
			Logger::Info("Created framebuffer ", handle);
			return *this;
		}
		void Destroy()
		{
			Logger::Info("Deleting framebuffer ", handle, ".");
			glDeleteFramebuffers(1, &handle);
		}
		Framebuffer& SetAttachments(FramebufferAttachments attachments)
		{
			clearBits = 0;
			if (attachments.depth.has_value())
				clearBits |= GL_DEPTH_BUFFER_BIT;
			if (!attachments.colors.empty())
				clearBits |= GL_COLOR_BUFFER_BIT;

			this->attachments = attachments;

			Bind();
			if (attachments.depth.has_value())
			{
				AddDepthTexture(attachments.depth.value().GetHandle());
				this->width = attachments.depth.value().GetWidth();
				this->height = attachments.depth.value().GetHeight();
			}
			else
			{
				this->width = attachments.colors[0].first.GetWidth();
				this->height = attachments.colors[0].first.GetHeight();
			}
			for (auto att : attachments.colors)
			{
				if (att.first.GetWidth() != width
					|| att.first.GetHeight() != height)
				{
					Logger::Error("A framebuffer's attachments should be of the same resolution");
				}
				AddColorTexture(att.first.GetHandle(), att.second);
			}
			return *this;
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
		FramebufferAttachments& GetAttachments() { return attachments; }
	protected:
		void AddColorTexture(GLuint texture, int idx)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, texture, 0);
		}
		void AddDepthTexture(GLuint texture)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
		}
	protected:
		int width, height;
		GLenum clearBits;
		FramebufferAttachments attachments;
	};
}

