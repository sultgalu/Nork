#pragma once

#include "TypedBuffers.h"
#include "VertexArrayWrapper.h"
#include "../Model/Material.h"
#include "../Model/Light.h"

namespace Nork::Renderer {
	struct DrawState
	{
	public:
		DrawState();
		std::shared_ptr<Material> AddMaterial();
		std::shared_ptr<DirLight> AddDirLight();
		std::shared_ptr<DirShadow> AddDirShadow(std::shared_ptr<Shader>, glm::uvec2 res, TextureFormat);
		std::shared_ptr<PointLight> AddPointLight();
		std::shared_ptr<PointShadow> AddPointShadow(std::shared_ptr<Shader>, uint32_t res, TextureFormat);
	public:
		MatrixUBO modelMatrixBuffer;
		MaterialUBO materialBuffer;

		DirLightUBO dirLightUBO;
		DirShadowUBO dirShadowUBO;
		PointLightUBO pointLightUBO;
		PointShadowUBO pointShadowUBO;

		VAO vaoWrapper;
		
	private:
		struct LightCount
		{
			uint32_t dirLight = 0, dirShadow = 0;
			uint32_t pointLight = 0, pointShadow = 0;
		};
		LightCount lightCount;
		std::shared_ptr<Buffer> lightCountUBO;
	};
}