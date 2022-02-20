#include "pch.h"
#include "Texture.h"

namespace Nork::Renderer {
	static GLuint boundTextures[32]{0};

	Texture& Nork::Renderer::Texture::Bind2D(int idx)
	{
		if (true || boundTextures[idx] != handle)
		{
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_2D, handle);
			boundTextures[idx] = handle;
		}
		return *this;
	}

	Texture& Nork::Renderer::Texture::BindCube(int idx)
	{
		if (true || boundTextures[idx] != handle)
		{
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
			boundTextures[idx] = handle;
		}
		return *this;
	}

	Texture2D& Texture2D::Bind(int idx)
	{
		Bind2D(idx);
		return *this;
	}
	TextureCube& TextureCube::Bind(int idx)
	{
		BindCube(idx);
		return *this;
	}
}