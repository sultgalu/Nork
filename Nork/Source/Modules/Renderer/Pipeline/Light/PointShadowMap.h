#pragma once

#include "../../Data/Lights.h"
#include "../../Model/DrawBatch.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Objects/Framebuffer/Framebuffer.h"

namespace Nork::Renderer {
	class PointShadowMap
	{
	public:
		PointShadowMap(std::shared_ptr<Shader> shader, std::shared_ptr<Framebuffer> framebuffer)
			: shader(shader), framebuffer(framebuffer)
		{}
		PointShadowMap(std::shared_ptr<Shader> shader, uint32_t size, TextureFormat depthFormat);
		void Render(const Data::PointLight& light, const Data::PointShadow& shadow, const std::vector<DrawCommandMultiIndirect>& drawCommands);
		std::shared_ptr<TextureCube> Get();
	private:
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Framebuffer> framebuffer;
	};
}

