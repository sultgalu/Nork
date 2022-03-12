#pragma once

#include "../GLObject.h"
#include "TextureParams.h"
#include "TextureFormat.h"
#include "TextureAttributes.h"

namespace Nork::Renderer {
	class Texture : public GLObject
	{
	public:
		Texture(GLuint handle, TextureParams params, TextureAttributes attributes)
			: GLObject(handle), params(params), attributes(attributes)
		{}
		~Texture()
		{
			Logger::Info("Deleting texture ", handle, ".");
			glDeleteTextures(1, &handle);
		}
		Texture& Bind2D(int idx = 0);
		Texture& BindCube(int idx = 0);
		const TextureAttributes& GetAttributes() { return attributes; }
		const TextureParams& GetParams() { return params; }
		uint32_t GetWidth() { return attributes.width; }
		uint32_t GetHeight() { return attributes.height; }
	protected:
		const TextureParams params;
		const TextureAttributes attributes;
	};

	class Texture2D: public Texture
	{
	public:
		using Texture::Texture;
		Texture2D& Bind(int idx = 0);
	};

	class TextureCube: public Texture
	{
	public:
		using Texture::Texture;
		TextureCube& Bind(int idx = 0);
	};
}

