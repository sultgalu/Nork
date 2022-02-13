#pragma once

enum class TextureTarget: uint8_t
{
	_2D = GL_TEXTURE_2D,
	_2DMS = GL_TEXTURE_2D_MULTISAMPLE,
	_2DArray = GL_TEXTURE_2D_ARRAY,
	Cube = GL_TEXTURE_CUBE_MAP
};

enum class TextureFilter
{
	Nearest = GL_NEAREST, Linear = GL_LINEAR, LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
	NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR, LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
	NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST
};

enum class TextureWrap
{
	Repeat = GL_REPEAT, MirrorRepeat = GL_MIRRORED_REPEAT, ClampToEdge = GL_CLAMP_TO_EDGE, ClampToBorder = GL_CLAMP_TO_BORDER
};

struct TextureParams
{
	TextureWrap wrap = TextureWrap::Repeat;
	TextureFilter filter = TextureFilter::LinearMipmapNearest;
	bool magLinear = true;
	bool genMipmap = false;
	static consteval TextureParams CubeMapParams()
	{
		return TextureParams{
			.wrap = TextureWrap::ClampToEdge,
			.filter = TextureFilter::Linear,
			.magLinear = false,
			.genMipmap = false
		};
	}
	static consteval TextureParams Tex2DParams()
	{
		return TextureParams{
			.wrap = TextureWrap::Repeat,
			.filter = TextureFilter::LinearMipmapNearest,
			.magLinear = true,
			.genMipmap = true
		};
	}

	static consteval TextureParams FramebufferTex2DParams()
	{
		return TextureParams{
			.wrap = TextureWrap::ClampToEdge,
			.filter = TextureFilter::Linear,
			.magLinear = true,
			.genMipmap = false
		};
	}

	GLenum GetWrap()
	{
		return std::to_underlying(wrap);
	}
	GLenum GetFilter()
	{
		return std::to_underlying(filter);
	}
};