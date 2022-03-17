#include "TextureBuilder.h"
#include "../GLManager.h"

namespace Nork::Renderer {
	std::shared_ptr<Texture2D> TextureBuilder::Create2D()
	{
		Create(false);
		auto tex = std::make_shared<Texture2D>(handle, params, attributes);
		Logger::Info("Created texture 2D ", handle, ".");
		GLManager::Get().texture2Ds[tex->GetHandle()] = tex;
		return tex;
	}
	std::shared_ptr<TextureCube> TextureBuilder::CreateCube()
	{
		Create(true);
		auto tex = std::make_shared<TextureCube>(handle, params, attributes);
		Logger::Info("Created texture Cube ", handle, ".");
		GLManager::Get().textureCubes[tex->GetHandle()] = tex;
		return tex;
	}
	void TextureBuilder::Create(bool cube)
	{
		Validate();
		glGenTextures(1, &handle);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, handle);
		SetParams(cube);
		SetData(cube);
	}
	void TextureBuilder::SetData(bool cube)
	{
		if (cube)
		{
			for (size_t i = 0; i < 6; i++)
			{
				Logger::Debug("Creating Cubemap face #", i);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, attributes.GetInternalFormat(), attributes.width, attributes.height, false, attributes.GetFormat(), attributes.GetType(), dataCube[i]);
			}
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, attributes.GetInternalFormat(), attributes.width, attributes.height, false, attributes.GetFormat(), attributes.GetType(), data2D);
		}
		if (params.genMipmap)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}
	void TextureBuilder::SetParams(bool cube)
	{
		glTexParameteri(cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.GetFilter());
		glTexParameteri(cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.magLinear ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.GetWrap());
		glTexParameteri(cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.GetWrap());
		if (cube)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, params.GetWrap());
		}
		if (params.shadow)
		{
			glTexParameteri(cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		}
	}
}
