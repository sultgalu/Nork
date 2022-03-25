#pragma once

#include "../Objects/Texture/Texture.h"

namespace Nork::Renderer::Model {
	struct Material
	{
		uint64_t diffuseMap;
		uint64_t normalsMap;
		uint64_t roughnessMap;
		uint64_t reflectMap;
		glm::vec3 diffuse;
		float specular;
		float specularExponent;
		float padding[3];
	};
}
namespace Nork::Renderer {
	enum class TextureMap: uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};

	struct Material
	{
		Material(uint32_t storageIdx);
	
		std::shared_ptr<Texture2D> diffuseMap;
		std::shared_ptr<Texture2D> normalsMap;
		std::shared_ptr<Texture2D> roughnessMap;
		std::shared_ptr<Texture2D> reflectMap;
		glm::vec3 diffuse;
		float specular;
		float specularExponent;
	
		Model::Material ToModel()
		{
			return Model::Material{
				.diffuseMap = diffuseMap->GetBindlessHandle(),
				.normalsMap = normalsMap->GetBindlessHandle(),
				.roughnessMap = roughnessMap->GetBindlessHandle(),
				.reflectMap = reflectMap->GetBindlessHandle(),
				.diffuse = diffuse,
				.specular = specular,
				.specularExponent = specularExponent
			};
		}
		uint32_t storageIdx;
		static std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)> GetDefaultTextureMaps();
	};

}