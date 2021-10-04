#pragma once

namespace Nork::Renderer::Utils
{
	namespace Shader
	{
		GLuint GetProgramFromSource(std::string_view src);
		std::unordered_map<std::string, GLint> GetUniforms(GLuint program);
		std::unordered_map<std::string, bool> GetMacros(const char* path);
		std::string SetMacros(std::string src, std::unordered_map<std::string, bool> macros);
	}

	namespace Texture
	{
		enum class Format
		{
			RGBA = GL_RGBA8, RGBA16F = GL_RGBA16F, RGB = GL_RGB8, RGB16F = GL_RGB16F, R32I = GL_R32I, R8 = GL_R8, R8I = GL_R8I, R32F = GL_R32F,
			Depth32F = GL_DEPTH_COMPONENT32F, Depth32 = GL_DEPTH_COMPONENT32, Depth24 = GL_DEPTH_COMPONENT24, Depth16 = GL_DEPTH_COMPONENT16,
			Depth24Stencil8 = GL_DEPTH24_STENCIL8, Depth32FStencil8 = GL_DEPTH32F_STENCIL8
		};

		inline GLenum GetInternalFormat(Format f) { return static_cast<GLenum>(f); }
		GLenum GetFormat(Format f);
		GLenum GetType(Format f);

		enum class Filter
		{
			Nearest = GL_NEAREST, Linear = GL_LINEAR, LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR, 
			NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR, LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
			NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST
		};

		enum class Wrap
		{
			Repeat = GL_REPEAT, MirrorRepeat = GL_MIRRORED_REPEAT, ClampToEdge = GL_CLAMP_TO_EDGE, ClampToBorder = GL_CLAMP_TO_BORDER 
		};

		std::vector<unsigned char> LoadImageData(std::string_view path, int& width, int& height, int& channels);
		unsigned int Create2D(const void* data, int width, int height, int channels, Wrap wrap = Wrap::Repeat, Filter filter = Filter::LinearMipmapNearest, bool magLinear = true, bool genMipmap = true);
		unsigned int Create2D(int width, int height, Format texFormat, int sampleCount = 1, bool repeat = false, void* data = nullptr, bool linear = true);
		unsigned int CreateCube(std::string dirPath, std::string extension);
		unsigned int CreateCube(int width, int height, Format texFormat, int sampleCount = 1);
		void Bind(unsigned int handle, int slot = 0);
		void BindMS(unsigned int handle, int slot = 0);
		void BindCube(unsigned int handle, int slot = 0);
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
			Builder& AddTexture(unsigned int* handler, Texture::Format format, GLenum attachment);
			Builder& AddRenderbuffer(unsigned int* handler, Texture::Format format, GLenum attachment);
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
		void ReadPixels(unsigned int fbo, int colorAtt, int x, int y, Texture::Format format, void* buf);
		unsigned int CreateUBO(int idx, int size, GLenum usage);
	}
}

