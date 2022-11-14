#include "pch.h"
#include "Texture.h"

namespace Nork::Renderer {
	static GLuint boundTextures[32]{0};

	const Texture& Nork::Renderer::Texture::Bind2D(int idx) const
	{
		if (true || boundTextures[idx] != handle)
		{
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_2D, handle);
			boundTextures[idx] = handle;
		}
		return *this;
	}

	const Texture& Nork::Renderer::Texture::BindCube(int idx) const
	{
		if (true || boundTextures[idx] != handle)
		{
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
			boundTextures[idx] = handle;
		}
		return *this;
	}

	std::vector<char> Texture::GetData2D() const
	{
		std::vector<char> data(Renderer::GetTexturePixelSize(attributes.format) * GetWidth() * GetHeight(), 0);
		glGetTexImage(GL_TEXTURE_2D, 0, GetTextureFormat(attributes.format), GetTextureType(attributes.format), data.data());
		return data;
	}

	const Texture2D& Texture2D::Bind(int idx) const
	{
		Bind2D(idx);
		return *this;
	}
	const TextureCube& TextureCube::Bind(int idx) const
	{
		BindCube(idx);
		return *this;
	}
}