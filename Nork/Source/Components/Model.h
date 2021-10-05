#pragma once

#include "Modules/Renderer/Data/Mesh.h"

namespace Nork::Components
{
	struct Model
	{
		using Mesh = Nork::Renderer::Data::Mesh;
		
		std::vector<Mesh> meshes;
	};
}