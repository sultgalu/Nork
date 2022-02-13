#pragma once

#include "Mesh.h"

namespace Nork::Renderer {
	struct Model
	{
		std::vector<Mesh> meshes;
		glm::mat4 modelMatrix;
	};
}