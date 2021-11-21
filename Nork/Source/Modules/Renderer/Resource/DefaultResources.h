#pragma once

#include "../Data/Texture.h"
#include "../Data/Mesh.h"

namespace Nork::Renderer
{
	class DefaultResources
	{
	public:
		static void Init();
		static void Free();

		inline static MeshResource cube;
		inline static TextureResource textures[textureTypeCount];
	};
}