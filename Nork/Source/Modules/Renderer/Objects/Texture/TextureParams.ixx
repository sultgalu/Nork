export module Nork.Renderer:TextureParams;

export enum class TextureTarget: uint8_t
{
	_2D = GL_TEXTURE_2D,
	_2DMS = GL_TEXTURE_2D_MULTISAMPLE,
	_2DArray = GL_TEXTURE_2D_ARRAY,
	Cube = GL_TEXTURE_CUBE_MAP
};

export enum class TextureFilter
{
	Nearest = GL_NEAREST, Linear = GL_LINEAR, LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
	NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR, LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
	NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
	None = GL_NONE
};

export enum class TextureWrap
{
	Repeat = GL_REPEAT, MirrorRepeat = GL_MIRRORED_REPEAT, ClampToEdge = GL_CLAMP_TO_EDGE, ClampToBorder = GL_CLAMP_TO_BORDER,
	None = GL_NONE
};

export struct TextureParams
{
	TextureWrap wrap = TextureWrap::None;
	TextureFilter filter = TextureFilter::None;
	bool magLinear = true;
	bool genMipmap = false;
	bool shadow = false;
	bool border = false;

	static consteval TextureParams CubeMapParams()
	{
		return TextureParams {
			.wrap = TextureWrap::ClampToEdge,
			.filter = TextureFilter::Linear,
			.genMipmap = false
		};
	}
	static consteval TextureParams Tex2DParams()
	{
		return TextureParams {
			.wrap = TextureWrap::Repeat,
			.filter = TextureFilter::LinearMipmapNearest,
			.genMipmap = true
		};
	}
	static consteval TextureParams FramebufferTex2DParams()
	{
		return TextureParams{
			.wrap = TextureWrap::ClampToEdge,
			.filter = TextureFilter::Linear,
			.genMipmap = false
		};
	}
	static consteval TextureParams ShadowMapParams2D()
	{
		return TextureParams{
			.wrap = TextureWrap::ClampToBorder,
			.filter = TextureFilter::Linear,
			.magLinear = true, // shadow anti-aliasing
			.genMipmap = false,
			.shadow = true,
			.border = true,
		};
	}
	static consteval TextureParams ShadowMapParamsCube()
	{
		return TextureParams{
			.wrap = TextureWrap::ClampToBorder,
			.filter = TextureFilter::Linear,
			.magLinear = true, // shadow anti-aliasing
			.genMipmap = false,
			.shadow = true,
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