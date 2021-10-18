#pragma once

namespace Nork::Renderer::Data
{
	struct TextureData
	{
		std::vector<unsigned char> data = {};
		uint16_t width = 0, height = 0;
		uint8_t channels = 0;
	};

	struct TextureResource
	{
		GLuint id = 0;
	};

	struct Texture
	{
		Texture(const TextureResource& res)
			: id(res.id) {}
		GLuint id;
	};
}