#pragma once

#include "TextureFormat.h"

namespace Nork::Renderer {
	// could use this to retrieve format + type, this would be 100% correct but would need to collect it to a map on startup
	/*
		glGetInternalformativ(GL_TEXTURE_2D, attributes.GetInternalFormat(), GL_GET_TEXTURE_IMAGE_TYPE, sizeof(preferredType), (GLint*)&preferredType);
		glGetInternalformativ(GL_TEXTURE_2D, attributes.GetInternalFormat(), GL_GET_TEXTURE_IMAGE_FORMAT, sizeof(preferredFormat), (GLint*)&preferredFormat);
	*/
	struct TextureAttributes
	{
		uint32_t width, height;
		TextureFormat format;
		GLenum GetType() const
		{
			return GetTextureType(format);
		}
		GLenum GetFormat() const
		{
			return GetTextureFormat(format);
		}
		GLenum GetInternalFormat() const
		{
			return static_cast<GLenum>(format);
		}
		uint32_t GetPixelSize() const
		{
			return GetTexturePixelSize(format);
		}
	};
}