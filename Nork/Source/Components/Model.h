#pragma once

#include "Modules/Renderer/Data/Mesh.h"

namespace Nork::Components
{
	struct Model
	{
		std::vector<Nork::Renderer2::Mesh> meshes;
	};
}