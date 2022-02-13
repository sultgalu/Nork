#pragma once

#include "Modules/Renderer2/Model/Mesh.h"

namespace Nork::Components
{
	struct Model
	{
		std::vector<Nork::Renderer::Mesh> meshes;
	};
}