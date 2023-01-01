#pragma once

#include "Mesh.h"
#include "Material.h"

namespace Nork::Renderer {
	struct Object
	{
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
		std::shared_ptr<BufferElement<glm::mat4>> modelMatrix;
	};
}