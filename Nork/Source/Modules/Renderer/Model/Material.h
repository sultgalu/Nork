#pragma once
#include "../Objects/Texture/Texture.h"
#include "../Objects/Texture/Texture.h"

namespace Nork::Renderer {
	enum class TextureMap: uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};

	namespace ShaderDefined {
		struct Material
		{
			uint64_t diffuseMap;
			uint64_t normalsMap;
			uint64_t roughnessMap;
			uint64_t reflectMap;
			glm::vec3 diffuse;
			float specular;
			float specularExponent;
		};
	}

	struct Material
	{
		Material();

		std::shared_ptr<Texture2D> diffuseMap = GetDefaultTextureMaps()[std::to_underlying(TextureMap::Diffuse)];
		std::shared_ptr<Texture2D> normalsMap = GetDefaultTextureMaps()[std::to_underlying(TextureMap::Normal)];
		std::shared_ptr<Texture2D> roughnessMap = GetDefaultTextureMaps()[std::to_underlying(TextureMap::Roughness)];
		std::shared_ptr<Texture2D> reflectMap = GetDefaultTextureMaps()[std::to_underlying(TextureMap::Reflection)];
		glm::vec3 diffuse;
		float specular;
		float specularExponent;

		ShaderDefined::Material ToShaderDefined()
		{
			return ShaderDefined::Material{
				.diffuseMap = diffuseMap->GetBindlessHandle(),
				.normalsMap = normalsMap->GetBindlessHandle(),
				.roughnessMap = roughnessMap->GetBindlessHandle(),
				.reflectMap = reflectMap->GetBindlessHandle(),
				.diffuse = diffuse,
				.specular = specular,
				.specularExponent = specularExponent
			};
		}
		static std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)> GetDefaultTextureMaps();
	};

}