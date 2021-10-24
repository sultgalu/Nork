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
		enum class TextureType : uint8_t
		{
			_2D, _2DMS, _2DArray, Cube
		};

		enum class Format: int
		{
			RGBA8 = GL_RGBA8, RGBA16F = GL_RGBA16F, RGBA32F = GL_RGBA32F, RGBA = RGBA8,
			RGB8 = GL_RGB8, RGB16F = GL_RGB16F, RGB32F = GL_RGB32F, RGB = RGB8,
			R32I = GL_R32I, R8 = GL_R8, R8I = GL_R8I, R32F = GL_R32F,
			Depth32F = GL_DEPTH_COMPONENT32F, Depth32 = GL_DEPTH_COMPONENT32, Depth24 = GL_DEPTH_COMPONENT24, Depth16 = GL_DEPTH_COMPONENT16,
			Depth24Stencil8 = GL_DEPTH24_STENCIL8, Depth32FStencil8 = GL_DEPTH32F_STENCIL8,
			None = GL_NONE
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

		template<TextureType Type>
		struct DataTypeOf
		{
		};

		template<>
		struct DataTypeOf<TextureType::Cube>
		{
			using type = const void**;
		};

		template<TextureType Type>
		static consteval GLenum GetTarget()
		{
			if constexpr (Type == TextureType::_2D)
			{
				return GL_TEXTURE_2D;
			}
			else if constexpr (Type == TextureType::Cube)
			{
				return GL_TEXTURE_CUBE_MAP;
			}
			else if constexpr (Type == TextureType::_2DMS)
			{
				return GL_TEXTURE_2D_MULTISAMPLE;
			}
			else if constexpr (Type == TextureType::_2DArray)
			{
				return GL_TEXTURE_2D_ARRAY;
			}
			else
			{
				MetaLogger().Error("Unhandled texture type here.");
				return GL_NONE;
			}
		}

		struct TextureParams
		{
			Wrap wrap = Wrap::Repeat;
			Filter filter = Filter::LinearMipmapNearest;
			bool magLinear = true;
			bool genMipmap = true;
			static consteval TextureParams CubeMapParams()
			{
				return TextureParams{
					.wrap = Wrap::ClampToEdge,
					.filter = Filter::Linear,
					.magLinear = false,
					.genMipmap = false 
				};
			}
		};

		std::vector<unsigned char> LoadImageData(std::string_view path, int& width, int& height, int& channels);
		template<TextureType Type = TextureType::_2D, typename DataPtr = void**>
		requires requires()
		{
			requires std::is_null_pointer<DataPtr>::value 
				|| (Type == TextureType::Cube && std::is_pointer<typename std::remove_pointer<DataPtr>::type>::value)
				|| (Type != TextureType::Cube && std::is_pointer<DataPtr>::value);
		}
		GLuint Create(uint32_t width, uint32_t height, Format format, DataPtr data = nullptr, TextureParams params = TextureParams())
		{
			unsigned int handle = 0;
			auto target = GetTarget<Type>();
			glGenTextures(1, &handle);
			glBindTexture(target, handle);

			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(params.filter));
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, params.magLinear ? GL_LINEAR : GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_WRAP_S, static_cast<GLenum>(params.wrap));
			glTexParameteri(target, GL_TEXTURE_WRAP_T, static_cast<GLenum>(params.wrap));
			if constexpr (Type == TextureType::Cube)
				glTexParameteri(target, GL_TEXTURE_WRAP_R, static_cast<GLenum>(params.wrap));

			if constexpr (Type == TextureType::_2D)
				glTexImage2D(GL_TEXTURE_2D, 0, GetInternalFormat(format), width, height, false, GetFormat(format), GetType(format), data);
			else if constexpr (Type == TextureType::Cube)
			{
				for (size_t i = 0; i < 6; i++)
				{
					Logger::Debug("Creating Cubemap face #", i);
					if(data == nullptr)
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetInternalFormat(format), width, height, false, GetFormat(format), GetType(format), nullptr);
					else
					{
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetInternalFormat(format), width, height, false, GetFormat(format), GetType(format), *((void**)data));
						data++;
					}
				}
			}
			else
			{
				MetaLogger().Error("This type of texture creaton is not implemented yet.");
			}

			if (params.genMipmap)
				glGenerateMipmap(target);

			return handle;
		}
		
		template<TextureType Type = TextureType::_2D>
		void Bind(unsigned int handle, int slot)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GetTarget<Type>(), handle);
		}
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
		std::vector<float> GetCubeVertexPositions8();
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

