#pragma once

#include "Stage.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Model/ShadowMap.h"
#include "../DrawCommands/DrawCommand.h"

namespace Nork::Renderer {
	struct ShadowMapProvider
	{
		virtual std::span<PointShadowMap> PointShadowMaps() = 0;
		virtual std::span<DirShadowMap> DirShadowMaps() = 0;
	};

	class ShadowMapStage : public Stage
	{
	public:
		ShadowMapStage(std::shared_ptr<Shader> dShader, std::shared_ptr<Shader> pShader, DrawCommand* drawCommand, ShadowMapProvider* provider)
			: dShader(dShader), pShader(pShader), drawCommand(drawCommand), provider(provider)
		{}
		bool Execute(Framebuffer& source, Framebuffer& destination) override;
		void RenderPointShadowMap(const PointShadowMap&);
		void RenderDirShadowMap(const DirShadowMap&);
	private:
	public:
		std::shared_ptr<Shader> dShader;
		std::shared_ptr<Shader> pShader;
		DrawCommand* drawCommand;
		ShadowMapProvider* provider;
	};
}