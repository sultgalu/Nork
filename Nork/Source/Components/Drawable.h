#pragma once

import Nork.Renderer;

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
		std::shared_ptr<Model> model;
		std::shared_ptr<glm::mat4*> modelMatrix;
	};
}