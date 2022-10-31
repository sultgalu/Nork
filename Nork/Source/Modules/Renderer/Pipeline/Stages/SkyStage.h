#pragma once

#include "Stage.h"
#include "../../Objects/Shader/Shader.h"

namespace Nork::Renderer {
	class SkyStage : public Stage
	{
	public:
		SkyStage(std::shared_ptr<Shader> shader);
		bool Execute(Framebuffer& source, Framebuffer& destination) override;
	public:
		std::shared_ptr<Shader> shader;
	};
}