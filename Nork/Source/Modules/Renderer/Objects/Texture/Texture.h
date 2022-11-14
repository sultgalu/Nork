#pragma once

#include "../GLObject.h"
#include "TextureParams.h"
#include "TextureFormat.h"
#include "TextureAttributes.h"

namespace Nork::Renderer {
	class Texture : public GLObject
	{
	public:
		Texture(GLuint handle, GLuint64 bindlessHandle, TextureParams params, TextureAttributes attributes)
			: GLObject(handle), bindlessHandle(bindlessHandle), params(params), attributes(attributes)
		{}
		~Texture()
		{
			Logger::Info("Deleting texture ", handle, ".");
			glDeleteTextures(1, &handle);
		}
		const Texture& Bind2D(int idx = 0) const;
		const Texture& BindCube(int idx = 0) const;
		GLuint64 GetBindlessHandle() const { return bindlessHandle; }
		const TextureAttributes& GetAttributes() const { return attributes; }
		const TextureParams& GetParams() const { return params; }
		uint32_t GetWidth() const { return attributes.width; }
		uint32_t GetHeight() const { return attributes.height; }
		std::vector<char> GetData2D() const;
	protected:
		const TextureParams params;
		const TextureAttributes attributes;
		const GLuint64 bindlessHandle;
	private:
		GLenum GetIdentifier() override
		{
			return GL_TEXTURE;
		}
	};

	class Texture2D: public Texture
	{
	public:
		using Texture::Texture;
		const Texture2D& Bind(int idx = 0) const;
	};

	class TextureCube: public Texture
	{
	public:
		using Texture::Texture;
		const TextureCube& Bind(int idx = 0) const;
	};
}

