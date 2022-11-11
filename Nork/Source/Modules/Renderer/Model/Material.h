#pragma once

#include "../Objects/Texture/Texture.h"
#include "../Data/Material.h"
#include "../Storage/SmartMappedBuffer.h"

namespace Nork::Renderer {
	enum class TextureMap: uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};

	struct Material
	{
		Material() = default;
		Material(SmartMappedBuffer<Data::Material>::Element);
		std::shared_ptr<Texture2D> GetTextureMap(TextureMap type) const
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
				material->diffuseMap = tex->GetBindlessHandle();
				break;
			case Normal:
				material->normalsMap = tex->GetBindlessHandle();
				break;
			case Roughness:
				material->roughnessMap = tex->GetBindlessHandle();
				break;
			case Reflection:
				material->reflectMap = tex->GetBindlessHandle();
				break;
			case COUNT:
				break;
			}
		}
		bool operator==(const Material& other) const { return other.material == material; }
		const Data::Material* operator->() const { return &material.Get(); }
		Data::Material* operator->() { return &material.Get(); }
		
		void SetDefaultTexture(TextureMap type);
		SmartMappedBuffer<Data::Material>::Element Element() { return material; }
		const SmartMappedBuffer<Data::Material>::Element Element() const { return material; }
	private:
		SmartMappedBuffer<Data::Material>::Element material;
		std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)> textureMaps;
	};

}