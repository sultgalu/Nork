#pragma once

#include "Modules/Renderer/Model/Model.h"

namespace Nork::Components
{
	struct Drawable
	{
		Renderer::Model model = Renderer::Model::Cube();
	};
}