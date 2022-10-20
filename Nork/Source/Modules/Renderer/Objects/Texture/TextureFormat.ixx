export module Nork.Renderer:TextureFormat;

export namespace Nork::Renderer {
	enum class TextureFormat : int
	{
		RGBA8 = GL_RGBA8, RGBA16F = GL_RGBA16F, RGBA32F = GL_RGBA32F, RGBA = RGBA8,
		RGB8 = GL_RGB8, RGB16F = GL_RGB16F, RGB32F = GL_RGB32F, RGB = RGB8,
		R32I = GL_R32I, R32UI = GL_R32UI, R32F = GL_R32F, R8 = GL_R8, R8I = GL_R8I,
		Depth32F = GL_DEPTH_COMPONENT32F, Depth32 = GL_DEPTH_COMPONENT32, Depth24 = GL_DEPTH_COMPONENT24, Depth16 = GL_DEPTH_COMPONENT16,
		Depth24Stencil8 = GL_DEPTH24_STENCIL8, Depth32FStencil8 = GL_DEPTH32F_STENCIL8,

		CompressedRGB8 = GL_COMPRESSED_RGB,

		None = GL_NONE
	};

	const char* TextureFormatToString(TextureFormat format)
	{
		using enum TextureFormat;
#define NORK_TEXTURE_FORMAT(f) case f: return #f
		switch (format)
		{
			NORK_TEXTURE_FORMAT(RGB8);
			NORK_TEXTURE_FORMAT(RGB16F);
			NORK_TEXTURE_FORMAT(RGB32F);
			NORK_TEXTURE_FORMAT(RGBA8);
			NORK_TEXTURE_FORMAT(RGBA16F);
			NORK_TEXTURE_FORMAT(RGBA32F);

			NORK_TEXTURE_FORMAT(R8);
			NORK_TEXTURE_FORMAT(R8I);
			NORK_TEXTURE_FORMAT(R32I);
			NORK_TEXTURE_FORMAT(R32UI);
			NORK_TEXTURE_FORMAT(R32F);

			NORK_TEXTURE_FORMAT(Depth16);
			NORK_TEXTURE_FORMAT(Depth24);
			NORK_TEXTURE_FORMAT(Depth32);
			NORK_TEXTURE_FORMAT(Depth32F);

			NORK_TEXTURE_FORMAT(Depth24Stencil8);
			NORK_TEXTURE_FORMAT(Depth32FStencil8);

			NORK_TEXTURE_FORMAT(CompressedRGB8);

			NORK_TEXTURE_FORMAT(None);
		default: return "...";
		}
	}

	GLenum GetTextureType(TextureFormat f)
	{
		using enum TextureFormat;

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

		case CompressedRGB8:
			return GL_BYTE;

		case None:
			MetaLogger().Error("Texture format \"None\" is an INVALID format.");

		default:
			Logger::Error("Unrecognized TextureFormat: ", (int)f);
			return GL_NONE;
		}
	}
	GLenum GetTextureFormat(TextureFormat f)
	{
		using enum TextureFormat;
		switch (f)
		{
		case RGBA8:
			return GL_RGBA;
		case RGBA16F:
			return GL_RGBA;
		case RGBA32F:
			return GL_RGBA;

		case RGB8:
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

		case CompressedRGB8:
			return GL_COMPRESSED_RGB;

		case None:
			MetaLogger().Error("Texture format \"None\" is an INVALID format.");

		default:
			Logger::Error("Unrecognized TextureFormat: ", (int)f);
			return GL_NONE;
		}
	}

	uint32_t GetTexturePixelSize(TextureFormat f)
	{
		using enum TextureFormat;
		switch (f)
		{
		case RGBA8:
			return 4;
		case RGBA16F:
			return 4 * 2;
		case RGBA32F:
			return 4 * 4;

		case RGB8:
			return 3;
		case RGB16F:
			return 3 * 2;
		case RGB32F:
			return 3 * 4;

		case R32I:
			return 4;
		case R32UI:
			return 4;
		case R8:
			return 1;
		case R8I:
			return 1;
		case R32F:
			return 4;

		case Depth32F:
			return 4;
		case Depth32:
			return 4;
		case Depth24:
			return 3;
		case Depth16:
			return 2;

		case Depth24Stencil8:
			return 4;
		case Depth32FStencil8:
			return 5;

		case CompressedRGB8:
			return 3;

		case None:
			MetaLogger().Error("Texture format \"None\" is an INVALID format.");

		default:
			Logger::Error("Unrecognized TextureFormat: ", (int)f);
			return GL_NONE;
		}
	}
	uint32_t GetTextureChannelCount(TextureFormat f)
	{
		using enum TextureFormat;
		switch (f)
		{
		case RGBA8:
			return 4;
		case RGBA16F:
			return 4;
		case RGBA32F:
			return 4;

		case RGB8:
			return 3;
		case RGB16F:
			return 3;
		case RGB32F:
			return 3;

		case R32I:
			return 1;
		case R32UI:
			return 1;
		case R8:
			return 1;
		case R8I:
			return 1;
		case R32F:
			return 1;

		case Depth32F:
			return 1;
		case Depth32:
			return 1;
		case Depth24:
			return 1;
		case Depth16:
			return 1;

		case Depth24Stencil8:
			return 2;
		case Depth32FStencil8:
			return 2;

		case CompressedRGB8:
			return 3;

		case None:
			MetaLogger().Error("Texture format \"None\" is an INVALID format.");

		default:
			Logger::Error("Unrecognized TextureFormat: ", (int)f);
			return GL_NONE;
		}
	}
}