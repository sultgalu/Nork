#pragma once

#include "TypedBufferWrapper.h"
#include "../Data/Material.h"
#include "../Data/Lights.h"
#include "../Data/Vertex.h"

namespace Nork::Renderer {
	using DefaultVBO = VBO<Data::Vertex>;

	using MaterialUBO = UBO<Data::Material>;
	using MatrixUBO = UBO<glm::mat4>;

	using DirLightUBO = UBO<Data::DirLight>;
	using DirShadowUBO = UBO<Data::DirShadow>;
	using PointLightUBO = UBO<Data::PointLight>;
	using PointShadowUBO = UBO<Data::PointShadow>;
}