#pragma once

#include "Material.h"

namespace Nork::Renderer {
	class MaterialBuilder
	{
	public:
		MaterialBuilder(MaterialBufferWrapper& buffer);
		MaterialBuilder& TextureMap(std::shared_ptr<Texture2D> tex, TextureMap mapType)
		{
			textureMaps[std::to_underlying(mapType)] = tex;
			return *this;
		}
		MaterialBuilder& Diffuse(const glm::vec3& color)
		{
			diffuse = color;
			return *this;
		}
		MaterialBuilder& Specular(float multiplier)
		{
			specular = multiplier;
			return *this;
		}
		MaterialBuilder& SpecularExponent(float exponent)
		{
			specularExponent = exponent;
			return *this;
		}
		std::shared_ptr<Material> Build();
	private:
		std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)> 
			textureMaps;
		glm::vec3 diffuse = { 1, 1, 1 };
		float specular = 1;
		float specularExponent = 128;

		MaterialBufferWrapper& buffer;
	};
}