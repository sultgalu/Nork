#pragma once

#include "../Lights.h"
#include "Modules/Renderer/Data/Ligths.h"

namespace Nork::Components::Converters
{
	inline Renderer::Data::DirLight GetDirLight(Components::DirLight dl)
	{
		return Renderer::Data::DirLight
		{
			.direction = dl.direction,
			.color = dl.color,
		};
	}

	inline Renderer::Data::PointLight GetPointLight(Components::PointLight pl)
	{
		return Renderer::Data::PointLight
		{
			.position = pl.position,
			.color = pl.color
		};
	}
}