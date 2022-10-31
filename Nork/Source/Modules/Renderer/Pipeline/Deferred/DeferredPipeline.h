#pragma once

#include "../../Objects/Framebuffer/GeometryFramebuffer.h"
#include "../../Objects/Framebuffer/LightFramebuffer.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Model/DrawBatch.h"

namespace Nork::Renderer {

	class DeferredPipeline
	{
	public:
		DeferredPipeline(std::shared_ptr<Texture2D> depth);
		void GeometryPass(Shader&, const std::vector<DrawCommandMultiIndirect>&);
		void LightPass(Shader&, MainFramebuffer&);
	public:
		std::shared_ptr<GeometryFramebuffer> geometryFb;
	};
}

