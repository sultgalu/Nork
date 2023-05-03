#include "Material.h"
#include "../Resources.h"

namespace Nork::Renderer {
	std::array<std::shared_ptr<Texture>, std::to_underlying(TextureMap::COUNT)> GetDefaultTextureMaps()
	{
		auto& textures = Resources::Instance().Textures();
		return { textures.diffuse, textures.normal, textures.metallicRoughness, textures.occlusion, textures.emissive };
	}
	Material::Material(std::shared_ptr<BufferElement<Data::Material>> deviceData)
		: DeviceDataProxy<Data::Material>(deviceData)
	{
		textureMaps = GetDefaultTextureMaps();
		for (size_t i = 0; i < textureMaps.size(); i++)
		{
			SetTextureMap(textureMaps[i], (TextureMap)i);
		}
		*deviceData = hostData;
	}
	void Material::SetDefaultTexture(TextureMap type)
	{
		SetTextureMap(GetDefaultTextureMaps()[std::to_underlying(type)], type);
	}
	bool Material::HasDefault(TextureMap type) const
	{
		return textureMaps[std::to_underlying(type)] == GetDefaultTextureMaps()[std::to_underlying(type)];
	}
}