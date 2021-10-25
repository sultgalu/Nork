#include "../Utils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb/stb_image.h>

namespace Nork::Renderer::Utils::Texture
{
	GLenum GetType(Format f)
	{
		using enum Format;

		switch (f)
		{
		case R8:
			return GL_BYTE;
		case R32F:
			return GL_FLOAT;
		case Depth32F:
			return GL_FLOAT;
		case RGBA32F:
			return GL_FLOAT;
		case RGB32F:
			return GL_FLOAT;
		case RGBA16F:
			return GL_HALF_FLOAT;
		case RGB16F:
			return GL_HALF_FLOAT;

		
		case R32I:
			return GL_INT;
		case R32UI:
			return GL_UNSIGNED_INT;
		case R8I:
			return GL_BYTE;

		case Depth32:
			return GL_UNSIGNED_INT;
		case Depth24:
			return GL_UNSIGNED_INT;
		case Depth16:
			return GL_UNSIGNED_SHORT;

		case RGBA8:
			return GL_UNSIGNED_BYTE;
		case RGB8:
			return GL_UNSIGNED_BYTE;


		case Depth24Stencil8:
			return GL_UNSIGNED_INT_24_8;
		case Depth32FStencil8:
			return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;

		case None:
			MetaLogger().Error("Texture format \"None\" is an INVALID format.");

		default:
			Logger::Error("Unrecognized TextureFormat: ", (int)f);
			return GL_NONE;
		}
	}
	GLenum GetFormat(Format f)
	{
		using enum Format;
		switch (f)
		{
		case RGBA:
			return GL_RGBA;
		case RGBA16F:
			return GL_RGBA;
		case RGBA32F:
			return GL_RGBA;

		case RGB:
			return GL_RGB;
		case RGB16F:
			return GL_RGB;
		case RGB32F:
			return GL_RGB;

		case R32I:
			return GL_RED_INTEGER;
		case R32UI:
			return GL_RED_INTEGER;
		case R8:
			return GL_RED;
		case R8I:
			return GL_RED_INTEGER;
		case R32F:
			return GL_RED;

		case Depth32F:
			return GL_DEPTH_COMPONENT;
		case Depth32:
			return GL_DEPTH_COMPONENT;
		case Depth24:
			return GL_DEPTH_COMPONENT;
		case Depth16:
			return GL_DEPTH_COMPONENT;

		case Depth24Stencil8:
			return GL_DEPTH_STENCIL;
		case Depth32FStencil8:
			return GL_DEPTH_STENCIL;

		case None:
			MetaLogger().Error("Texture format \"None\" is an INVALID format.");

		default:
			Logger::Error("Unrecognized TextureFormat: ", (int)f);
			return GL_NONE;
		}
	}
	
	std::vector<unsigned char> LoadImageData(std::string_view path, int& width, int& height, int& channels)
	{
		unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 0);
		size_t size = (size_t)width * height * channels;
		if (data)
		{
			std::vector<unsigned char> result;
			result.assign(data, data + size);
			stbi_image_free(data);

			return result;
		}
		else
		{
			Logger::Error("Failed to load texture data from ", path);
			return std::vector<unsigned char>();
		}
	}
	
	/*unsigned int Create2D(const void* data, int width, int height, int channels, Wrap wrap, Filter filter, bool magLinear, bool genMipmap)
	{
		unsigned int handle = 0;
		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_2D, handle);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(filter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magLinear ? GL_LINEAR : GL_NEAREST);

		GLenum format;
		if (channels == 3) [[likely]]
			format = GL_RGB;
		else if (channels == 4)
			format = GL_RGBA;
		else if (channels == 1)
			format = GL_RED;
		else [[unlikely]]
		{
			Logger::Error(channels, " number of channels is not supported");
			return 0;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, false, format, GL_UNSIGNED_BYTE, data);
		if (genMipmap)
			glGenerateMipmap(GL_TEXTURE_2D);

		return handle;
	}
	unsigned int Create2D(int width, int height, Format texFormat, int sampleCount, bool repeat, void* data, bool linear)
	{
		unsigned int handle = 0;
		glGenTextures(1, &handle);
		glBindTexture(sampleCount < 2 ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, handle);

		GLenum iFormat = GetInternalFormat(texFormat);
		GLenum format = GetFormat(texFormat);
		GLenum type = GetType(texFormat);

		if (sampleCount < 2)
		{
			if (false && data == nullptr)
				glTexStorage2D(GL_TEXTURE_2D, 1, iFormat, width, height);
			else
				glTexImage2D(GL_TEXTURE_2D, 0, iFormat, width, height, false, format, type, data);
		}
		else
		{
			if (data == nullptr)
				glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, iFormat, width, height, GL_TRUE);
			else
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, iFormat, width, height, GL_TRUE);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);

		return handle;
	}
	unsigned int CreateCube(int width, int height, Format texFormat, int sampleCount)
	{
		unsigned int id;
		glGenTextures(1, &id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetInternalFormat(texFormat),
				width, height, 0, GetFormat(texFormat), GetType(texFormat), 0);
		}

		return id;
	}*/
	/*GLuint LoadCubemap(std::string dirPath, std::string extension)
	{
		static std::string suffixes[6]{
			"right", "left","top","bottom","front","back",
		};

		if (dirPath.at(dirPath.size() - 1) != '/')
			dirPath.append("/");
		std::vector<std::vector<unsigned char>> datas;
		int width, height, nrChannels;

		for (int i = 0; i < 6; i++)
		{
			auto data = LoadImageData((dirPath + suffixes[i] + extension).c_str(), width, height, nrChannels);
			datas.push_back(data);

			if (data.size() > 0)
			{
				Logger::Debug("Loading Cubemap face #", i);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, false, GL_RGB, GL_UNSIGNED_BYTE, data.data());
			}
			else
			{
				std::cout << "ERR::FAILED TO LOAD TEXTURE" << std::endl;
				std::abort();
			}
		}
		std::vector<void*> pointers;
		for (size_t i = 0; i < datas.size(); i++)
		{
			pointers.push_back(datas[i].data());
		}
		return Create<TextureType::Cube>(width, height, Format::RGB8, pointers.data(), TextureParams(Wrap::ClampToEdge, Filter::Linear, false, false));
	}*/
}