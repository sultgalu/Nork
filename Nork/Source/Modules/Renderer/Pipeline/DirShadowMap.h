#pragma once

#include "../Model/Lights.h"
#include "../Model/Model.h"
#include "../Objects/Shader/Shader.h"
#include "../Objects/Framebuffer/Framebuffer.h"

namespace Nork::Renderer {

	class DirShadowMap
	{
	public:
		DirShadowMap(std::shared_ptr<Shader> shader, std::shared_ptr<Framebuffer> framebuffer)
			: shader(shader), framebuffer(framebuffer)
		{}
		DirShadowMap(std::shared_ptr<Shader> shader, uint32_t width, uint32_t height, TextureFormat depthFormat)
			: shader(shader)
		{
			auto depth = TextureBuilder()
				.Params(TextureParams::FramebufferTex2DParams())
				.Attributes(TextureAttributes{ .width = width, .height = height, .format = depthFormat })
				.Create2DEmpty();
			framebuffer = FramebufferBuilder().Attachments(FramebufferAttachments().Depth(depth)).Create();
		}
		void Render(const DirLight& light, const DirShadow& shadow, ModelIterator iterator);
		void Bind(const DirShadow& shadow);
		std::shared_ptr<Texture2D> Get();
	private:
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Framebuffer> framebuffer;
	};
}

