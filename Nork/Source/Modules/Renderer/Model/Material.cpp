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
	std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMap::COUNT)> GetDefaultTextureMaps()
	{
		static auto diff = CreateTexture2D(TextureFormat::RGBA32F, { 1.0f, 1.0f, 1.0f, 1.0f });
		static auto norm = CreateTexture2D(TextureFormat::RGB32F, { 0.5f, 0.5f, 1.0f });
		static auto refl = CreateTexture2D(TextureFormat::R32F, { 0.5f });
		static auto rough = CreateTexture2D(TextureFormat::R32F, { 0.5f });
		return { diff, norm, rough, refl };
	}
	Material::Material(std::shared_ptr<Data::Material*> ptr)
		: ptr(ptr)
	{
		textureMaps = GetDefaultTextureMaps();

		diffuseMap = textureMaps[std::to_underlying(TextureMap::Diffuse)]->GetBindlessHandle();
		normalsMap = textureMaps[std::to_underlying(TextureMap::Normal)]->GetBindlessHandle();
		roughnessMap = textureMaps[std::to_underlying(TextureMap::Roughness)]->GetBindlessHandle();
		reflectMap = textureMaps[std::to_underlying(TextureMap::Reflection)]->GetBindlessHandle();
		diffuse = { 1, 1, 1 };
		specular = 1;
		specularExponent = 128;
		Update();
	}
	void Material::SetDefaultTexture(TextureMap type)
	{
		SetTextureMap(GetDefaultTextureMaps()[std::to_underlying(type)], type);
	}
}