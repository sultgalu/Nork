#pragma once

#include "TextureFormat.h"

namespace Nork::Renderer {
	struct TextureAttributes
	{
		uint32_t width, height;
		TextureFormat format;
		GLenum GetType()
		{
			return GetTextureType(format);
		}
		GLenum GetFormat()
		{
			return GetTextureFormat(format);
		}
		GLenum GetInternalFormat()
		{
			return static_cast<GLenum>(format);
		}
	};
}