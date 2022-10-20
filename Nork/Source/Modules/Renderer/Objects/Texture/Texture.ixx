export module Nork.Renderer:Texture;

export import :GLObject;
export import :TextureParams;
export import :TextureFormat;
export import :TextureAttributes;

export namespace Nork::Renderer {
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
		Texture& Bind2D(int idx = 0);
		Texture& BindCube(int idx = 0);
		GLuint64 GetBindlessHandle() { return bindlessHandle; }
		const TextureAttributes& GetAttributes() { return attributes; }
		const TextureParams& GetParams() { return params; }
		uint32_t GetWidth() { return attributes.width; }
		uint32_t GetHeight() { return attributes.height; }
		void GetData2D(void*);
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
		Texture2D& Bind(int idx = 0);
	};

	class TextureCube: public Texture
	{
	public:
		using Texture::Texture;
		TextureCube& Bind(int idx = 0);
	};
}

