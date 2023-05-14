#pragma once

#include "Modules/Renderer/Model/Material.h"
#include "Modules/Renderer/Model/Mesh.h"

namespace Nork::Components
{

struct Drawable
{
	std::shared_ptr<Renderer::Object> object;
};
}