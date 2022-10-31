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
		GLenum ClearBits() const
		{
			return (depth == nullptr ? 0 : GL_DEPTH_BUFFER_BIT) | (colors.empty() ? 0 : GL_COLOR_BUFFER_BIT);
		}
	};

	class Framebuffer: public GLObject
	{
	public:
		Framebuffer(GLuint handle, int width, int height, FramebufferAttachments attachments)
			: GLObject(handle), width(width), height(height), attachments(attachments)
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
		Framebuffer& Bind(GLenum target = GL_FRAMEBUFFER)
		{
			glBindFramebuffer(target, handle);
			return *this;
		}
		Framebuffer& SetViewport()
		{
			glViewport(0, 0, width, height);
			return *this;
		}
		Framebuffer& Clear()
		{
			glClear(attachments.ClearBits());
			return *this;
		}
		Framebuffer& ClearColor()
		{
			if (attachments.ClearBits() & GL_COLOR_BUFFER_BIT)
				glClear(GL_COLOR_BUFFER_BIT);
			return *this;
		}
		Framebuffer& ClearDepth()
		{
			if (attachments.ClearBits() & GL_DEPTH_BUFFER_BIT)
				glClear(GL_DEPTH_BUFFER_BIT);
			return *this;
		}
		std::shared_ptr<Texture> Color(int idx = 0)
		{
			for (auto& att : attachments.colors)
			{
				if (att.second == idx)
					return att.first;
			}
			return nullptr;
		}
		std::shared_ptr<Texture> Depth()
		{
			return attachments.depth;
		}
		int Width() { return width; }
		int Height() { return height; }
		const FramebufferAttachments& GetAttachments() { return attachments; }
	protected:
		const int width, height;
		const FramebufferAttachments attachments;
	private:
		GLenum GetIdentifier() override
		{
			return GL_FRAMEBUFFER;
		}
	};
}

