#pragma once

#include "LightFramebuffer.h"
#include "FramebufferBuilder.h"

namespace Nork::Renderer {

	class LightFramebufferBuilder : FramebufferBuilder
	{
	public:
		LightFramebufferBuilder& Width(int width)
		{
			this->width = width;
			return *this;
		}
		LightFramebufferBuilder& Height(int height)
		{
			this->height = height;
			return *this;
		}
		LightFramebufferBuilder& ColorFormat(TextureFormat color)
		{
			this->color = color;
			return *this;
		}
		LightFramebufferBuilder& DepthFormat(TextureFormat depth)
		{
			this->depth = depth;
			return *this;
		}
		std::shared_ptr<MainFramebuffer> Create();
	private:
		void Validate()
		{
			if (depth == TextureFormat::None || color == TextureFormat::None)
			{
				std::abort();
			}
		}
		void CreateAttachments();
	private:
		TextureFormat depth = TextureFormat::None;
		TextureFormat color = TextureFormat::None;
	};
}