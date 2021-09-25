#pragma once

namespace Nork::Renderer::Utils
{
	namespace Shader
	{
		GLuint GetProgramFromSource(std::string_view src);
		std::unordered_map<std::string_view, GLint> GetUniforms(GLuint program);
		std::unordered_map<std::string_view, bool> GetMacros(const char* path);
		std::string SetMacros(std::string_view src, std::unordered_map<std::string_view, bool> macros);
	}

	namespace Texture
	{
		enum class TextureFormat
		{
			RGBA = GL_RGBA8, RGBA16F = GL_RGBA16F, RGB = GL_RGB8, RGB16F = GL_RGB16F, R32I = GL_R32I, R8 = GL_R8, R8I = GL_R8I, R32F = GL_R32F,
			Depth32F = GL_DEPTH_COMPONENT32F, Depth32 = GL_DEPTH_COMPONENT32, Depth24 = GL_DEPTH_COMPONENT24, Depth16 = GL_DEPTH_COMPONENT16,
			Depth24Stencil8 = GL_DEPTH24_STENCIL8, Depth32FStencil8 = GL_DEPTH32F_STENCIL8
		};

		unsigned int CreateTexture2D(int width, int height, TextureFormat texFormat, int sampleCount = 1, bool repeat = false, void* data = nullptr, bool linear = true);
		unsigned int CreateTexture2D(std::string path, bool repeat = true);
		unsigned int CreateTextureCube(std::string dirPath, std::string extension);
		unsigned int CreateTextureCube(int width, int height, TextureFormat texFormat, int sampleCount = 1);
		void BindTexture(unsigned int handle, int slot = 0);
		void BindTextureMS(unsigned int handle, int slot = 0);
		void BindTextureCube(unsigned int handle, int slot = 0);

		GLenum GetType(TextureFormat f);
		GLenum GetFormat(TextureFormat f);
		GLenum GetInternalFormat(TextureFormat f);
	}

	namespace VAO
	{
		class Builder
		{
		public:
			Builder();
			Builder& AddBuffer(unsigned int* handler, GLenum target, GLenum usage, int size, void* data);
			Builder& SetAttribs(std::vector<int> attrLens);
			unsigned int GetVertexArrayBuffer();
		private:
			unsigned int vao = 0;
		};
	}

	namespace Framebuffer
	{
		class Builder
		{
		public:
			Builder(int width, int height);
			Builder& AddTexture(unsigned int handler, GLenum attachment);
			Builder& AddTexture(unsigned int* handler, Texture::TextureFormat format, GLenum attachment);
			Builder& AddRenderbuffer(unsigned int* handler, Texture::TextureFormat format, GLenum attachment);
			unsigned int GetFramebuffer(bool assertComplete = true);
		private:
			int width, height;
			unsigned int fbo = 0;
		};
	}

	namespace Draw
	{
		void Quad();
		void Cubemap();
	}

	namespace Mesh
	{
		std::vector<unsigned int> GetCubeIndices();
		std::vector<float> GetCubeVertexPositions();
		std::vector<float> GetCubeVertexTexCoords();
		std::vector<float> GetCubeVertexNormals();
		std::vector<float> GetCubeVertexTangents();
		std::vector<float> GetCubeVertexBitangents();
	}

	namespace Other
	{
		void ReadPixels(unsigned int fbo, int colorAtt, int x, int y, Texture::TextureFormat format, void* buf);
		unsigned int CreateUBO(int idx, int size, GLenum usage);
	}
}

