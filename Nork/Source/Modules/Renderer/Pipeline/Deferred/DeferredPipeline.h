#pragma once

#include "../../Objects/Framebuffer/GeometryFramebuffer.h"
#include "../../Objects/Framebuffer/LightFramebuffer.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Model/Model.h"

namespace Nork::Renderer {

	class DeferredPipeline
	{
	public:
		DeferredPipeline(std::shared_ptr<GeometryFramebuffer> geometryFb, std::shared_ptr<LightFramebuffer> lightFb,
			std::shared_ptr<Shader> geomatryShader, std::shared_ptr<Shader> lightShader)
			: geometryFb(geometryFb), lightFb(lightFb), geomatryShader(geomatryShader), lightShader(lightShader)
		{}
		void GeometryPass(ModelIterator iterator);
		void LightPass();
	public:
		std::shared_ptr<GeometryFramebuffer> geometryFb;
		std::shared_ptr<LightFramebuffer> lightFb;
		std::shared_ptr<Shader> geomatryShader, lightShader;
	};
}

