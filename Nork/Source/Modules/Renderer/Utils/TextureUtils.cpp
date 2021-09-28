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
			return GL_FLOAT;
		case R32F:
			return GL_FLOAT;
		case Depth32F:
			return GL_FLOAT;
		case RGBA16F:
			return GL_FLOAT;
		case RGB16F:
			return GL_FLOAT;

		
		case R32I:
			return GL_INT;

		case R8I:
			return GL_BYTE;

		case Depth32:
			return GL_UNSIGNED_INT;
		case Depth24:
			return GL_UNSIGNED_INT;
		case Depth16:
			return GL_UNSIGNED_INT;

		case RGBA:
			return GL_RGBA;
		case RGB:
			return GL_RGB;


		case Depth24Stencil8:
			return GL_UNSIGNED_INT_24_8;
		case Depth32FStencil8:
			return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;

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
		case RGB:
			return GL_RGB;
		case RGB16F:
			return GL_RGB;

		case R32I:
			return GL_RED;
		case R8:
			return GL_RED;
		case R8I:
			return GL_RED_INTEGER;
		case R32F:
			return GL_RED_INTEGER;

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
			auto result = std::vector<unsigned char>(/*data, data + size*/);
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
	unsigned int CreateTexture2D(const void* data, int width, int height, int channels, Wrap wrap, Filter filter, bool magLinear, bool genMipmap)
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
	unsigned int CreateTexture2D(int width, int height, Format texFormat, int sampleCount, bool repeat, void* data, bool linear)
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
	unsigned int CreateTextureCube(std::string dirPath, std::string extension)
	{
		unsigned int id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		static std::string suffixes[6]{
			"right", "left","top","bottom","front","back",
		};

		for (int i = 0; i < 6; i++)
		{
			int width, height, nrChannels;
			unsigned char* data = stbi_load((dirPath + suffixes[i] + extension).c_str(), &width, &height, &nrChannels, 0);

			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, false, GL_RGB, GL_UNSIGNED_BYTE, data);
				//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "ERR::FAILED TO LOAD TEXTURE" << std::endl;
				std::abort();
			}
		}

		return id;
	}
	unsigned int CreateTextureCube(int width, int height, Format texFormat, int sampleCount)
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
	}
	void BindTexture(unsigned int handle, int slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, handle);
	}
	void BindTextureMS(unsigned int handle, int slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, handle);
	}
	void BindTextureCube(unsigned int handle, int slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
	}
}