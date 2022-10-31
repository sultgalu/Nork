#pragma once

#include "GeometryFramebuffer.h"

namespace Nork::Renderer {
	// TODO: rename to MainFramebuffer or smt, it has the structure of a main pipeline framebuffer that is the source/target for every render stage
	class MainFramebuffer : public Framebuffer
	{
	public:
		MainFramebuffer(GLuint handle, int width, int height, FramebufferAttachments attachments)
			: Framebuffer(handle, width, height, attachments)
		{
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