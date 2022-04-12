#pragma once

#include "../../Objects/Framebuffer/Framebuffer.h"
#include "../../Objects/Buffer/Buffer.h"
#include "../../Objects/Shader/Shader.h"

namespace Nork::Renderer {
	class Bloom
	{
	public:
		Bloom();
		void InitTextures(uint32_t size, uint32_t baseX, uint32_t baseY);
		void Apply(std::shared_ptr<Texture2D>, std::shared_ptr<Shader>, std::shared_ptr<Shader>, std::shared_ptr<Shader>);

		std::vector<std::shared_ptr<Framebuffer>> fbs;
		std::vector<std::shared_ptr<Framebuffer>> fbs2;
		std::shared_ptr<Framebuffer> dest;
		std::shared_ptr<Buffer> ubo;
	};
}