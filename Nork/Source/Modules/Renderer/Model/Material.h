#pragma once

import Nork.Renderer;
#include "../Data/Material.h"
#include "../Storage/TypedBuffers.h"

namespace Nork::Renderer {
	enum class TextureMap: uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};

	struct Material : Data::Material
	{
		Material(std::shared_ptr<Data::Material*> ptr);
		void Update()
		{
			**ptr = *(Data::Material*)this;
		}
		std::shared_ptr<Texture2D> GetTextureMap(TextureMap type)
		{
			return textureMaps[std::to_underlying(type)];
		}
		void SetTextureMap(std::shared_ptr<Texture2D> tex, TextureMap type)
		{
			textureMaps[std::to_underlying(type)] = tex;
			using enum Nork::Renderer::TextureMap;
			switch (type)
			{
			case Diffuse:
				diffuseMap = tex->GetBindlessHandle();
				break;
			case Normal:
				normalsMap = tex->GetBindlessHandle();
				break;
			case Roughness:
				roughnessMap = tex->GetBindlessHandle();
				break;
			case Reflection:
				reflectMap = tex->GetBindlessHandle();
				break;
			case COUNT:
				break;
			}
		}
		void SetDefaultTexture(TextureMap type);
		std::shared_ptr<Data::Material*> GetPtr() { return ptr; }
	private:
		std::shared_ptr<Data::Material*> ptr;
		std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)> textureMaps;
	};

}