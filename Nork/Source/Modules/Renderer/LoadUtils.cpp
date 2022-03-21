#include "LoadUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb/stb_image.h>

namespace Nork::Renderer
{
	static TextureFormat GetFormat(int channels)
	{
		using enum TextureFormat;
		switch (channels)
		{
		case 4: [[unlikely]]
			return RGBA8;
		case 3: [[likely]]
			return RGB8;
		case 1: [[ads]]
			return R8;
		default:
			MetaLogger().Error("Unhandled number of channels");
		}
	}
    Image LoadUtils::LoadImage(std::string_view path)
    {
        int width = 0, height = 0, channels = 0;

		unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 0);
		size_t size = (size_t)width * height * channels;
		if (data)
		{
			//auto d = std::vector<char>(data, data + size);
			auto image = Image{
				.width = (uint32_t)width,
				.height = (uint32_t)height,
				.channels = (uint32_t)channels,
				.format = GetFormat(channels),
				.data = std::vector<char>()
			};
			image.data.assign(data, data + size);
			std::memcpy(image.data.data(), data, size);
			stbi_image_free(data);
			return image;
		}
		else
		{
			Logger::Error("Failed to load texture data from ", path);
			return Image{
				.width = 1, .height = 1, .channels = 1, .format = TextureFormat::R8, .data = std::vector<char>(1)
			};
		}
    }
	std::array<Image, 6> LoadUtils::LoadCubemapImages(std::string dirPath, std::string extension)
	{
		static std::string suffixes[6]{
			"right", "left","top","bottom","front","back",
		};

		if (dirPath.at(dirPath.size() - 1) != '/')
			dirPath.append("/");

		std::array<Image, 6> datas;
		int width, height, nrChannels;

		for (int i = 0; i < 6; i++)
		{
			datas[i] = LoadImage(dirPath + suffixes[i] + extension);
		}
		return datas;
	}
    std::string LoadShader(std::string_view path)
    {
        std::ifstream stream(path.data());
        std::stringstream buf;
        buf << stream.rdbuf();

		return buf.str();
    }

	std::vector<MeshData> LoadUtils::LoadModel(std::string path)
	{
		return {};
	}
}
