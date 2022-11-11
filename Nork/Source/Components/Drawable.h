#pragma once

#include "Modules/Renderer/World.h"

namespace Nork {
}

namespace Nork::Components
{
	struct Mesh
	{
		Renderer::Mesh mesh; // assigne only with entt::patch
		Renderer::Material material; // assigne only with entt::patch
	};

	struct Model
	{
		std::vector<Mesh> meshes;
	};

	struct Drawable
	{
		std::shared_ptr<Model> model;
		Renderer::ModelMatrix modelMatrix;
	};
}