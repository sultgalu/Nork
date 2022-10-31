#pragma once

#include "Stage.h"
#include "../../Objects/Shader/Shader.h"
#include "../../LoadUtils.h"

namespace Nork::Renderer {
	class SkyboxStage : public Stage
	{
	public:
		SkyboxStage(std::shared_ptr<Shader>, const std::array<Image, 6>& images);
		bool Execute(Framebuffer& source, Framebuffer& destination) override;
	public:
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Renderer::TextureCube> skybox;
	};
}