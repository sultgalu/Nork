#pragma once

#include "../Model/Lights.h"
#include "../Model/Model.h"
#include "../Objects/Shader/Shader.h"
#include "../Objects/Framebuffer/Framebuffer.h"

namespace Nork::Renderer {
	class PointShadowMap
	{
	public:
		PointShadowMap(std::shared_ptr<Shader> shader, std::shared_ptr<Framebuffer> framebuffer)
			: shader(shader), framebuffer(framebuffer)
		{}
		PointShadowMap(std::shared_ptr<Shader> shader, uint32_t size, TextureFormat depthFormat)
			: shader(shader)
		{
			auto depth = TextureBuilder()
				.Params(TextureParams::CubeMapParams())
				.Attributes(TextureAttributes{ .width = size, .height = size, .format = depthFormat })
				.CreateCubeEmpty();
			framebuffer = FramebufferBuilder().Attachments(FramebufferAttachments().Depth(depth)).Create();
		}
		void Render(const PointLight& light, const PointShadow& shadow, ModelIterator iterator);
		void Bind(const PointShadow& shadow);
		std::shared_ptr<TextureCube> Get();
	private:
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Framebuffer> framebuffer;
	};
}

