#pragma once

#include "Mesh.h"
#include "Material.h"

namespace Nork::Renderer {
	struct Object
	{
		Mesh mesh;
		Material material;
		//UBO<glm::mat4>::Element modelMatrix;
	};
}