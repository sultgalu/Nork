#pragma once

#include "Modules/Renderer/Model/Mesh.h"

namespace Nork::Components
{
	struct Model
	{
		std::vector<Nork::Renderer::Mesh> meshes;
	};
}