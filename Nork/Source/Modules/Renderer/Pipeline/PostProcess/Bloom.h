#pragma once

#include "../../Objects/Framebuffer/Framebuffer.h"
#include "../../Objects/Buffer/Buffer.h"
#include "../../Objects/Shader/Shader.h"

namespace Nork::Renderer {
	class Bloom
	{
	public:
		Bloom();
		void InitTextures();
		void Apply(std::shared_ptr<Texture2D>, std::shared_ptr<Shader>, std::shared_ptr<Shader>, std::shared_ptr<Shader>);

		std::vector<std::shared_ptr<Framebuffer>> fbs;
		std::vector<std::shared_ptr<Framebuffer>> fbs2;
		std::shared_ptr<Framebuffer> dest;
		std::shared_ptr<Buffer> ubo;

		float divider = 1.33f;
		float highResY = 1080;
		float lowResY = 100;
		float ratio = 16.f / 9.f;
	};
}