#include "Material.h"
#include "../Objects/Texture/TextureBuilder.h"

namespace Nork::Renderer {
	static std::shared_ptr<Texture2D> CreateTexture2D(TextureFormat format, std::vector<float> data)
	{
		return TextureBuilder()
			.Params(TextureParams::Tex2DParams())
			.Attributes(TextureAttributes{ .width = 1, .height = 1, .format = format })
			.Create2DWithData(data.data());
	}
	Material::Material(uint32_t storageIdx)
		: storageIdx(storageIdx)
	{
		diffuseMap = GetDefaultTextureMaps()[std::to_underlying(TextureMap::Diffuse)];
		normalsMap = GetDefaultTextureMaps()[std::to_underlying(TextureMap::Normal)];
		roughnessMap = GetDefaultTextureMaps()[std::to_underlying(TextureMap::Roughness)];
		reflectMap = GetDefaultTextureMaps()[std::to_underlying(TextureMap::Reflection)];
		diffuse = { 1, 1, 1 };
		specular = 1;
		specularExponent = 128;
	}
	std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)> Material::GetDefaultTextureMaps()
	{
		static auto diff = CreateTexture2D(TextureFormat::RGBA32F, { 1.0f, 1.0f, 1.0f, 1.0f });
		static auto norm = CreateTexture2D(TextureFormat::RGB32F, { 0.5f, 0.5f, 1.0f });
		static auto refl = CreateTexture2D(TextureFormat::R32F, { 0.5f });
		static auto rough = CreateTexture2D(TextureFormat::R32F, { 0.5f });
		return { diff, norm, refl, rough };
	}
}