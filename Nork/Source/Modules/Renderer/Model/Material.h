#pragma once

#include "../Objects/Texture/Texture.h"
#include "../Data/Material.h"
#include "../Storage/TypedBufferWrapper.h"

namespace Nork::Renderer {
	using MaterialBufferWrapper = TypedBufferWrapper<Data::Material, BufferTarget::UBO>;
	
	enum class TextureMap: uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};

	struct Material
	{
		Material(MaterialBufferWrapper& buffer, std::shared_ptr<size_t> bufferIdx, std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)>
			textureMaps, glm::vec3 diffuse,	float specular, float specularExponent)
			: buffer(buffer), bufferIdx(bufferIdx),
			diffuseMap(textureMaps[std::to_underlying(TextureMap::Diffuse)]),
			normalsMap(textureMaps[std::to_underlying(TextureMap::Normal)]),
			roughnessMap(textureMaps[std::to_underlying(TextureMap::Roughness)]),
			reflectMap(textureMaps[std::to_underlying(TextureMap::Reflection)]),
			diffuse(diffuse), specular(specular), specularExponent(specularExponent)
		{}

		std::shared_ptr<Texture2D> diffuseMap;
		std::shared_ptr<Texture2D> normalsMap;
		std::shared_ptr<Texture2D> roughnessMap;
		std::shared_ptr<Texture2D> reflectMap;
		glm::vec3 diffuse;
		float specular;
		float specularExponent;
	
		Data::Material ToData()
		{
			return Data::Material{
				.diffuseMap = diffuseMap->GetBindlessHandle(),
				.normalsMap = normalsMap->GetBindlessHandle(),
				.roughnessMap = roughnessMap->GetBindlessHandle(),
				.reflectMap = reflectMap->GetBindlessHandle(),
				.diffuse = diffuse,
				.specular = specular,
				.specularExponent = specularExponent
			};
		}
		size_t GetBufferIndex() { return *bufferIdx; }
		void Update()
		{
			buffer.Update(bufferIdx, ToData());
		}
	private:
		std::shared_ptr<size_t> bufferIdx;
		MaterialBufferWrapper& buffer;
	};

}