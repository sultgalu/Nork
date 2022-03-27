#pragma once

#include "../../Data/Lights.h"
#include "../../Model/DrawBatch.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Objects/Framebuffer/Framebuffer.h"

namespace Nork::Renderer {

	class DirShadowMap
	{
	public:
		DirShadowMap(std::shared_ptr<Shader> shader, std::shared_ptr<Framebuffer> framebuffer)
			: shader(shader), framebuffer(framebuffer)
		{}
		DirShadowMap(std::shared_ptr<Shader> shader, uint32_t width, uint32_t height, TextureFormat depthFormat);
		void Render(const Data::DirLight& light, const Data::DirShadow& shadow, const std::vector<DrawCommandMultiIndirect>& drawCommands);
		std::shared_ptr<Texture2D> Get();
	private:
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Framebuffer> framebuffer;
	};
}

