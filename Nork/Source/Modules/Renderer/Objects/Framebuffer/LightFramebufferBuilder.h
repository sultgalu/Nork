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
		LightFramebufferBuilder& DepthTexture(std::shared_ptr<Texture2D> depth)
		{
			this->depth = depth;
			return *this;
		}
		std::shared_ptr<LightFramebuffer> Create();
	private:
		void Validate()
		{
			if (depth == nullptr || color == TextureFormat::None)
			{
				std::abort();
			}
		}
		void CreateAttachments();
	private:
		std::shared_ptr<Texture2D> depth = nullptr;
		TextureFormat color = TextureFormat::None;
	};
}