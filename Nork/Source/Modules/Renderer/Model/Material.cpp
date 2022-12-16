#include "Material.h"

namespace Nork::Renderer {
	/*static std::shared_ptr<Texture2D> CreateTexture2D(TextureFormat format, std::vector<float> data)
	{
		return TextureBuilder()
			.Params(TextureParams::Tex2DParams())
			.Attributes(TextureAttributes{ .width = 1, .height = 1, .format = format })
			.Create2DWithData(data.data());
	}
	std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)> GetDefaultTextureMaps()
	{
		static auto diff = CreateTexture2D(TextureFormat::RGBA32F, { 1.0f, 1.0f, 1.0f, 1.0f });
		static auto norm = CreateTexture2D(TextureFormat::RGB32F, { 0.5f, 0.5f, 1.0f });
		static auto rough = CreateTexture2D(TextureFormat::RGB32F, { 0.0f, 1.0f, 1.0f }); // g=roughness, b=metallic
		return { diff, norm, rough };
	}
	Material::Material(SmartMappedBuffer<Data::Material>::Element element)
		: material(element)
	{
		textureMaps = GetDefaultTextureMaps();

		material->baseColor = textureMaps[std::to_underlying(TextureMap::BaseColor)]->GetBindlessHandle();
		material->normal = textureMaps[std::to_underlying(TextureMap::Normal)]->GetBindlessHandle();
		material->metallicRoughness = textureMaps[std::to_underlying(TextureMap::MetallicRoughness)]->GetBindlessHandle();
	}
	void Material::SetDefaultTexture(TextureMap type)
	{
		SetTextureMap(GetDefaultTextureMaps()[std::to_underlying(type)], type);
	}
	bool Material::HasDefault(TextureMap type) const
	{
		return textureMaps[std::to_underlying(type)] == GetDefaultTextureMaps()[std::to_underlying(type)];
	}*/
}