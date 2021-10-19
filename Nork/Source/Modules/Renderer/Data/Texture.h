#pragma once

#include "../Utils.h"

namespace Nork::Renderer::Data
{
	struct TextureData
	{
		std::vector<unsigned char> data = {};
		uint16_t width = 0, height = 0;
		uint8_t channels = 0;
		Utils::Texture::Format format;
	};

	struct TextureResource
	{
		GLuint id = 0;
	};

	/*struct Texture
	{
		Texture(const TextureResource& res)
			: id(res.id) {}
		GLuint id;
	};*/

	template<Utils::Texture::TextureType type>
	struct Texture
	{
	public:
		Texture(TextureResource& res)
			: id (res.id)
		{
		}
		inline void Bind(uint8_t slot = 0)
		{
			Utils::Texture::Bind(id, slot);
		}
		inline GLuint Id() { return id; }
	protected:
		GLuint id;
	};
}