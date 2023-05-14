#pragma once

#include "../Data/Material.h"
#include "DeviceData.h"
#include "../Image.h"

namespace Nork::Renderer {

struct Texture
{
	std::shared_ptr<Image> image;
	const uint32_t descriptorIdx;
	~Texture();
};
enum class TextureMap : uint8_t
{
	BaseColor = 0, Normal, MetallicRoughness, Occlusion, Emissive, COUNT
};
enum class ShadingMode : uint8_t {
	Default = 0, Blend, Unlit
};
struct Material : DeviceDataProxy<Data::Material>
{
	Material(std::shared_ptr<BufferElement<Data::Material>>);
	std::shared_ptr<Texture> GetTextureMap(TextureMap type) const
	{
		return textureMaps[std::to_underlying(type)];
	}
	bool HasDefault(TextureMap type) const;
	void SetTextureMap(const std::shared_ptr<Texture>& tex, TextureMap type);
	void SetDefaultTexture(TextureMap type);
public:
	std::array<std::shared_ptr<Texture>, std::to_underlying(TextureMap::COUNT)> textureMaps;
	bool blending = false; // to keep the gltf blending option
};

}