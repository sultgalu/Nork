#pragma once

namespace Nork::Renderer {
	enum class TextureFormat : int
	{
		RGBA8 = GL_RGBA8, RGBA16F = GL_RGBA16F, RGBA32F = GL_RGBA32F, RGBA = RGBA8,
		RGB8 = GL_RGB8, RGB16F = GL_RGB16F, RGB32F = GL_RGB32F, RGB = RGB8,
		R32I = GL_R32I, R32UI = GL_R32UI, R32F = GL_R32F, R8 = GL_R8, R8I = GL_R8I,
		Depth32F = GL_DEPTH_COMPONENT32F, Depth32 = GL_DEPTH_COMPONENT32, Depth24 = GL_DEPTH_COMPONENT24, Depth16 = GL_DEPTH_COMPONENT16,
		Depth24Stencil8 = GL_DEPTH24_STENCIL8, Depth32FStencil8 = GL_DEPTH32F_STENCIL8,
		None = GL_NONE
	};

	static GLenum GetTextureType(TextureFormat f)
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

		case None:
			MetaLogger().Error("Texture format \"None\" is an INVALID format.");

		default:
			Logger::Error("Unrecognized TextureFormat: ", (int)f);
			return GL_NONE;
		}
	}
	static GLenum GetTextureFormat(TextureFormat f)
	{
		using enum TextureFormat;
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
}