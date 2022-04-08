#pragma once

#include "Modules/Renderer/Model/Mesh.h"
#include "Modules/Renderer/Model/Material.h"

namespace Nork {
}

namespace Nork::Components
{
	struct Mesh
	{
		std::shared_ptr<Renderer::Mesh> mesh;
		std::shared_ptr<Renderer::Material> material;
	};

	struct Model
	{
		std::vector<Mesh> meshes;
	};

	struct Drawable
	{
		Model model;
		std::shared_ptr<glm::mat4*> modelMatrix;
	};
}