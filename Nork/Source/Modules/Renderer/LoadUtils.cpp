#include "LoadUtils.h"

#define STB_IMAGE_IMPLEMENTATION
//#define STBI_NO_BMP
//#define STBI_NO_PSD
//#define STBI_NO_TGA
//#define STBI_NO_GIF
//#define STBI_NO_HDR
//#define STBI_NO_PIC
//#define STBI_NO_PNM
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include <stb_image_write.h>

namespace Nork::Renderer
{
ImageData LoadUtils::LoadImage(std::string_view path, bool forceRGBA)
{
	int width = 0, height = 0, channels = 0;

	unsigned char* data = stbi_load(path.data(), &width, &height, &channels, forceRGBA ? STBI_rgb_alpha : 0);
	size_t size = (size_t)width * height * (forceRGBA ? 4 : channels);
	if (data)
	{
		auto image = ImageData{
			.width = (uint32_t)width,
			.height = (uint32_t)height,
			.channels = (uint32_t)channels,
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
		return ImageData{
			.width = 1, .height = 1, .channels = 1, .data = std::vector<char>(1)
		};
	}
}
static int WriteImage_stb(const ImageData& image, const std::string& path, ImageFormat format)
{
	switch (format)
	{
		case ImageFormat::JPEG:
			return stbi_write_jpg(path.c_str(), image.width, image.height, image.channels, image.data.data(), 0);
		case ImageFormat::PNG:
			return stbi_write_png(path.c_str(), image.width, image.height, image.channels, image.data.data(), 0);
		case ImageFormat::BMP:
			return stbi_write_bmp(path.c_str(), image.width, image.height, image.channels, image.data.data());
		default:
			std::unreachable();
	}
}
void LoadUtils::WriteImage(const ImageData& image, const std::string& path, ImageFormat format)
{
	if (WriteImage_stb(image, path, format) == 0)
		Logger::Error("Failed to write image to ", path);
}
/*void LoadUtils::WriteTexture(const Renderer::Texture2D& tex, const std::string& path, Renderer::ImageFormat format)
{
	Renderer::Image image;
	image.format = tex.GetAttributes().format;
	image.width = tex.GetWidth();
	image.height = tex.GetHeight();
	image.data = tex.Bind().GetData2D();
	image.channels = image.data.size() / (image.width * image.height);
	WriteImage(image, path, format);
}*/
std::array<ImageData, 6> LoadUtils::LoadCubemapImages(std::string dirPath, std::string extension)
{
	static std::string suffixes[6]{
		"right", "left","top","bottom","front","back",
	};

	if (dirPath.at(dirPath.size() - 1) != '/')
		dirPath.append("/");

	std::array<ImageData, 6> datas;
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
}
