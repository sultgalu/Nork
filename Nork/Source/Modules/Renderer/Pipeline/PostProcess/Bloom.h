#pragma once

#include "../../Objects/Framebuffer/LightFramebuffer.h"
#include "../../Objects/Buffer/Buffer.h"
#include "../../Objects/Shader/Shader.h"

namespace Nork::Renderer {
	class Bloom
	{
	public:
		Bloom(uint32_t width = 1080);
		void Apply(std::shared_ptr<MainFramebuffer>, std::shared_ptr<Shader>, std::shared_ptr<Shader>, std::shared_ptr<Shader>);

		std::vector<std::shared_ptr<Framebuffer>> fbs;
		std::vector<std::shared_ptr<Framebuffer>> fbs2;
		std::shared_ptr<Texture2D> blackTex;

		float divider = 1.33f;
		float highResY;
		float lowResY = 100;
		float ratio = 16.f / 9.f;
	};
}