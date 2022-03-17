#pragma once

#include "GeometryFramebuffer.h"

namespace Nork::Renderer {
	class LightFramebuffer : public Framebuffer
	{
	public:
		LightFramebuffer(GLuint handle, int width, int height, FramebufferAttachments attachments)
			: Framebuffer(handle, width, height, attachments)
		{
			clearBits = GL_COLOR_BUFFER_BIT;
		}
		std::shared_ptr<Texture2D> Color()
		{
			return std::static_pointer_cast<Texture2D>(attachments.colors[0].first);
		}
		std::shared_ptr<Texture2D> Depth()
		{
			return std::static_pointer_cast<Texture2D>(attachments.depth);
		}
	};
}