#include "pch.h"
#include "Texture.h"

namespace Nork::Renderer2 {
	static GLuint boundTextures[32]{0};

	Texture2D& Texture2D::Bind(int idx)
	{
		if (true || boundTextures[idx] != handle)
		{
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_2D, handle);
			boundTextures[idx] = handle;
		}
		return *this;
	}
	TextureCube& TextureCube::Bind(int idx)
	{
		if (true || boundTextures[idx] != handle)
		{
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
			boundTextures[idx] = handle;
		}
		return *this;
	}
}