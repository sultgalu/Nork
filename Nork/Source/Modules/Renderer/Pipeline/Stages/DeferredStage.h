#pragma once

#include "../../Objects/Framebuffer/GeometryFramebuffer.h"
#include "../../Objects/Shader/Shader.h"
#include "../DrawCommands/DrawCommand.h"
#include "Stage.h"

namespace Nork::Renderer {
	class DeferredStage : public Stage
	{
	public:
		DeferredStage(std::shared_ptr<Texture2D> depth, std::shared_ptr<Shader> gShader, std::shared_ptr<Shader> lShader, DrawCommand*);
		bool Execute(Framebuffer& source, Framebuffer& destination) override;
	private:
		void GeometryPass();
		void LightPass(Framebuffer&);
	public:
		std::shared_ptr<GeometryFramebuffer> geometryFb;
		std::shared_ptr<Shader> gShader;
		std::shared_ptr<Shader> lShader;
		const DrawCommand* drawCommand;
	};
}