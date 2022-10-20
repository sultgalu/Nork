module Nork.Renderer;

namespace Nork::Renderer {
	static std::vector<int> QuerySupportedCompressedFormats()
	{
		int count = 0;
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &count);
		std::vector<int> list(count, 0);

		glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, list.data());
		Logger::Info("List of supported texture formats for compressed storage:");
		for (auto& iFormat : list)
		{
			auto format = static_cast<TextureFormat>((GLenum)iFormat);
			Logger::Info(TextureFormatToString(format));
		}
		return list;
	}
	static std::vector<int> GetSupportedCompressedFormats()
	{
		static auto list = QuerySupportedCompressedFormats();
		return list;
	}
	static bool IsSupportedForCompression(GLenum internalFormat)
	{
		for (auto& supported : GetSupportedCompressedFormats())
		{
			if (supported == internalFormat)
				return true;
		}
		return false;
	}
	std::shared_ptr<Texture2D> TextureBuilder::Create2D()
	{
		Create(false);
		auto tex = std::make_shared<Texture2D>(handle, bindlessHandle, params, attributes);
		Logger::Info("Created texture 2D ", handle, ".");
		GLManager::Get().texture2Ds[tex->GetHandle()] = tex;
		return tex;
	}
	std::shared_ptr<TextureCube> TextureBuilder::CreateCube()
	{
		Create(true);
		auto tex = std::make_shared<TextureCube>(handle, bindlessHandle, params, attributes);
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
		bindlessHandle = glGetTextureHandleARB(handle);
		glMakeTextureHandleResidentARB(bindlessHandle);
	}
	void TextureBuilder::SetData(bool cube)
	{
		if (cube)
		{
			for (size_t i = 0; i < 6; i++)
			{
				Logger::Debug("Creating Cubemap face #", i);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, attributes.GetInternalFormat(), attributes.width, attributes.height, 0, attributes.GetFormat(), attributes.GetType(), dataCube[i]);
			}
		}
		else
		{
			if (IsSupportedForCompression(attributes.GetInternalFormat()))
			{
				Logger::Warning("Compressed formats are not supported yet. This will probably fail");
				glTexImage2D(GL_TEXTURE_2D, 0, attributes.GetInternalFormat(), attributes.width, attributes.height, 0, attributes.GetFormat(), attributes.GetType(), data2D);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, attributes.GetInternalFormat(), attributes.width, attributes.height, 0, attributes.GetFormat(), attributes.GetType(), data2D);
			}
		}
		if (params.genMipmap)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}
	void TextureBuilder::SetParams(bool cube)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
		if (params.border)
		{
			glm::vec4 border(1, 1, 1, 1);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &border.x);
		}
	}
}
