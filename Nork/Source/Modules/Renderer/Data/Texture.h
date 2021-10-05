#pragma once

namespace Nork::Renderer::Data
{
	enum class TextureType : uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};
	constinit const uint8_t textureTypeCount = static_cast<uint8_t>(TextureType::COUNT);

	struct TextureData
	{
		std::vector<unsigned char> data;
		uint16_t width, height;
		uint8_t channels;
	};

	struct TextureResource
	{
		GLuint id;
	};

	struct Texture
	{
		Texture(const TextureResource& res)
			: id(res.id) {}
		GLuint id;
	};
}