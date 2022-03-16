#pragma once

#include "../../Objects/Framebuffer/GeometryFramebuffer.h"
#include "../../Objects/Framebuffer/LightFramebuffer.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Model/Model.h"

namespace Nork::Renderer {

	class DeferredPipeline
	{
	public:
		DeferredPipeline(std::shared_ptr<Shader> geomatryShader, std::shared_ptr<Shader> lightShader, 
			std::shared_ptr<GeometryFramebuffer> geometryFb, std::shared_ptr<LightFramebuffer> lightFb)
			: geomatryShader(geomatryShader), lightShader(lightShader), geometryFb(geometryFb), lightFb(lightFb)
		{}
		DeferredPipeline(std::shared_ptr<Shader> geomatryShader, std::shared_ptr<Shader> lightShader,
			uint32_t width, uint32_t height);
		void GeometryPass(ModelIterator iterator);
		void LightPass();
	public:
		std::shared_ptr<GeometryFramebuffer> geometryFb;
		std::shared_ptr<LightFramebuffer> lightFb;
		std::shared_ptr<Shader> geomatryShader, lightShader;
	};
}

