#pragma once

#include "../Data/Texture.h"
#include "../Data/Mesh.h"

namespace Nork::Renderer::Resource
{
	class DefaultResources
	{
	public:
		static void Init();
		static void Free();

		inline static Data::MeshResource cube;
		inline static Data::TextureResource textures[Data::textureTypeCount];
	};
}