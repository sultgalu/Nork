#pragma once

#include "../Data/Material.h"
#include "../Resources.h"
#include "DeviceData.h"

namespace Nork::Renderer {
	enum class TextureMap: uint8_t
	{
		BaseColor = 0, Normal, MetallicRoughness, Occlusion, Emissive, COUNT
	};
	enum class ShadingMode: uint8_t {
		Default = 0, Blend, Emissive
	};
	struct Material: DeviceDataProxy<Data::Material>
	{
		Material(std::shared_ptr<BufferElement<Data::Material>>);
		std::shared_ptr<Texture> GetTextureMap(TextureMap type) const
		{
			return textureMaps[std::to_underlying(type)];
		}
		bool HasDefault(TextureMap type) const;
		void SetTextureMap(const std::shared_ptr<Texture>& tex, TextureMap type)
		{
			textureMaps[std::to_underlying(type)] = tex;
			using enum Nork::Renderer::TextureMap;
			switch (type)
			{
			case BaseColor:
				hostData.baseColor = tex->descriptorIdx;
				break;
			case Normal:
				hostData.normal = tex->descriptorIdx;
				break;
			case MetallicRoughness:
				hostData.metallicRoughness = tex->descriptorIdx;
				break;
			case Occlusion:
				hostData.occlusion = tex->descriptorIdx;
				break;
			case Emissive:
				hostData.emissive = tex->descriptorIdx;
				break;
			case COUNT:
				break;
			}
		}
		void SetDefaultTexture(TextureMap type);
	public:
		std::array<std::shared_ptr<Texture>, std::to_underlying(TextureMap::COUNT)> textureMaps;
		ShadingMode shadingMode = ShadingMode::Default;
	};

}