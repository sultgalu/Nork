#include "Material.h"
#include "../Resources.h"

namespace Nork::Renderer {
	static std::shared_ptr<Texture> CreateTexture2D(vk::Format format, std::vector<float> data)
	{
		auto texImg = std::make_shared<Image>(1, 1, format,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
			vk::PipelineStageFlagBits2::eVertexShader, vk::AccessFlagBits2::eShaderSampledRead);

		texImg->Write(data.data(), data.size() * sizeof(data[0]), vk::ImageLayout::eShaderReadOnlyOptimal);
		return Resources::Instance().Textures().AddTexture(texImg);
	}
	std::array<std::shared_ptr<Texture>, std::to_underlying(TextureMap::COUNT)> GetDefaultTextureMaps()
	{
		static auto diff = CreateTexture2D(Vulkan::Format::rgba32f, { 1.0f, 1.0f, 1.0f, 1.0f });
		static auto norm = CreateTexture2D(Vulkan::Format::rgba32f, { 0.5f, 0.5f, 1.0f, 0.0f }); // 'a' unused
		static auto rough = CreateTexture2D(Vulkan::Format::rgba32f, { 0.0f, 1.0f, 1.0f, 0.0f }); // g=roughness, b=metallic
		return { diff, norm, rough };
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