#pragma once

#include "../Data/Material.h"

namespace Nork::Renderer {
	enum class TextureMap: uint8_t
	{
		BaseColor = 0, Normal, MetallicRoughness, COUNT
	};

	struct Material
	{
		Material() = default;
		/*Material(SmartMappedBuffer<Data::Material>::Element);
		std::shared_ptr<Texture2D> GetTextureMap(TextureMap type) const
		{
			return textureMaps[std::to_underlying(type)];
		}
		bool HasDefault(TextureMap type) const;
		void SetTextureMap(std::shared_ptr<Texture2D> tex, TextureMap type)
		{
			textureMaps[std::to_underlying(type)] = tex;
			using enum Nork::Renderer::TextureMap;
			switch (type)
			{
			case BaseColor:
				material->baseColor = tex->GetBindlessHandle();
				break;
			case Normal:
				material->normal = tex->GetBindlessHandle();
				break;
			case MetallicRoughness:
				material->metallicRoughness = tex->GetBindlessHandle();
				break;
			case COUNT:
				break;
			}
		}*/
		// bool operator==(const Material& other) const { return other.material == material; }
		// const Data::Material* operator->() const { return &material.Get(); }
		// Data::Material* operator->() { return &material.Get(); }
		
		void SetDefaultTexture(TextureMap type);
		private:
		//SmartMappedBuffer<Data::Material>::Element material;
		//std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)> textureMaps;
	};

}