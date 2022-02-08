#pragma once

#include "../GLObject.h"
#include "TextureParams.h"
#include "TextureFormat.h"
#include "TextureAttributes.h"

namespace Nork::Renderer2 {
	class Texture2D: public GLObject
	{
	public:
		Texture2D& Create()
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &handle);
			Logger::Info("Created texture ", handle, ".");
			return *this;
		}
		void Destroy()
		{
			Logger::Info("Deleting texture ", handle, ".");
			glDeleteTextures(1, &handle);
		}
		Texture2D& SetParams(TextureParams params = TextureParams::Tex2DParams())
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.GetFilter());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.magLinear ? GL_LINEAR : GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.GetWrap());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.GetWrap());
			this->params = params;
			return *this;
		}
		Texture2D& SetData(TextureAttributes attribs, void* data = nullptr)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, attribs.GetInternalFormat(), attribs.width, attribs.height, false, attribs.GetFormat(), attribs.GetType(), data);
			if (params.genMipmap)
				glGenerateMipmap(GL_TEXTURE_2D);

			this->attributes = attribs;
			return *this;
		}
		Texture2D& Bind(int idx = 0);
		const TextureAttributes& Attributes() { return attributes; }
	private:
		TextureParams params;
		TextureAttributes attributes;
	};

	class TextureCube: public GLObject
	{
	public:
		TextureCube& Create()
		{
			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &handle);
			Logger::Info("Created texture ", handle, ".");
			return *this;
		}
		void Destroy()
		{
			Logger::Info("Deleting texture ", handle, ".");
			glDeleteTextures(1, &handle);
		}
		TextureCube& SetParams(TextureParams params = TextureParams::CubeMapParams())
		{
			Bind();

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, params.GetFilter());
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, params.magLinear ? GL_LINEAR : GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, params.GetWrap());
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, params.GetWrap());
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, params.GetWrap());

			this->params = params;
			return *this;
		}
		TextureCube& SetData(TextureAttributes attribs, void** dataArray = nullptr)
		{
			for (size_t i = 0; i < 6; i++)
			{
				Logger::Debug("Creating Cubemap face #", i);

				void* data = dataArray == nullptr ? nullptr : dataArray[i];
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, attribs.GetInternalFormat(), attribs.width, attribs.height, false, attribs.GetFormat(), attribs.GetType(), data);
			}

			if (params.genMipmap)
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

			this->attributes = attribs;
			return *this;
		}
		TextureCube& Bind(int idx = 0);
	private:
		TextureParams params;
		TextureAttributes attributes;
	};
}

