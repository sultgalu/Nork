#pragma once

#include "Modules/Renderer/Data/Mesh.h"

namespace Nork::Components
{
	struct Model
	{
		using Mesh = Nork::Renderer::Mesh;
		
		std::vector<Mesh> meshes;
	};
}