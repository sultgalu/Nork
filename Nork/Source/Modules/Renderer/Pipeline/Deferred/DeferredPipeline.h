#pragma once

#include "../../State/Capabilities.h"
#include "../../Objects/Framebuffer/Framebuffer.h"
#include "../../Objects/Buffer/Buffer.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Objects/GLManager.h"
#include "../../Model/Model.h"
#include "../../DrawUtils.h"

namespace Nork::Renderer {

	class DeferredPipeline
	{
	public:
		static void GeometryPass(GeometryFramebuffer& geometryFb, Shader& shader, ModelIterator iterator);
		static void LightPass(GeometryFramebuffer& geometryFb, LightFramebuffer& lightFb, Shader& shader);
	};
}

