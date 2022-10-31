#pragma once

#include "../../Objects/Framebuffer/LightFramebuffer.h"
#include "../../Objects/Buffer/Buffer.h"
#include "../../Objects/Shader/Shader.h"
#include "Stage.h"

namespace Nork::Renderer {
	class BloomStage : public Stage
	{
	public:
		BloomStage(std::shared_ptr<Shader> filterShader, std::shared_ptr<Shader> downscaleShader, std::shared_ptr<Shader> upscaleShader, uint32_t height = 1080);
		bool Execute(Framebuffer& source, Framebuffer& destination) override;
		
		std::vector<std::shared_ptr<Framebuffer>> fbs;
		std::vector<std::shared_ptr<Framebuffer>> fbs2;

		std::shared_ptr<Shader> filterShader;
		std::shared_ptr<Shader> downscaleShader;
		std::shared_ptr<Shader> upscaleShader;

		float divider = 1.33f;
		float highResY;
		float lowResY = 100;
		float ratio = 16.f / 9.f;
	};
}