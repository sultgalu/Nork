#pragma once

#include "Mesh.h"

namespace Nork::Renderer2 {
	struct Model
	{
		std::vector<Mesh> meshes;
		glm::mat4 modelMatrix;
	};
}