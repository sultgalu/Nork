#pragma once

#include "../Data/Texture.h"

namespace Nork::Renderer::Resource
{
	class DefaultResources
	{
	public:
		static void Init();
		static void Free();
		inline static unsigned int textures[Data::textureTypeCount];
	};
}