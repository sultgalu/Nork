#pragma once

namespace Nork::Renderer {
	enum class TextureTarget : uint8_t
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
		NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
		None = GL_NONE
	};

	enum class TextureWrap
	{
		Repeat = GL_REPEAT, MirrorRepeat = GL_MIRRORED_REPEAT, ClampToEdge = GL_CLAMP_TO_EDGE, ClampToBorder = GL_CLAMP_TO_BORDER,
		None = GL_NONE
	};

	struct TextureParams
	{
		TextureWrap wrap = TextureWrap::None;
		TextureFilter filter = TextureFilter::None;
		bool magLinear = true;
		bool genMipmap = false;
		bool shadow = false;
		bool border = false;

		static TextureParams CubeMapParams();
		static TextureParams Tex2DParams();
		static TextureParams FramebufferTex2DParams();
		static TextureParams ShadowMapParams2D();
		static TextureParams ShadowMapParamsCube();

		GLenum GetWrap()
		{
			return std::to_underlying(wrap);
		}
		GLenum GetFilter()
		{
			return std::to_underlying(filter);
		}
	};
}