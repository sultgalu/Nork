#pragma once

#include "Buffer/Buffer.h"
#include "VertexArray/VertexArray.h"
#include "Texture/Texture.h"
#include "Framebuffer/Framebuffer.h"
#include "Framebuffer/GeometryFramebuffer.h"
#include "Framebuffer/LightFramebuffer.h"
#include "Shader/Shader.h"

namespace Nork::Renderer {

	class GLManager
	{
	public:
		std::unordered_map<GLuint, std::weak_ptr<Texture2D>> texture2Ds;
		std::unordered_map<GLuint, std::weak_ptr<TextureCube>> textureCubes;
		std::unordered_map<GLuint, std::weak_ptr<Buffer>> buffers;
		std::unordered_map<GLuint, std::weak_ptr<VertexArray>> vaos;
		std::unordered_map<GLuint, std::weak_ptr<Framebuffer>> fbos;
		std::unordered_map<GLuint, std::weak_ptr<Shader>> shaders;
	};

	static GLManager glManager;

	class BufferBuilder
	{
	public:
		BufferBuilder& Data(void* data, size_t size)
		{
			this->data = data;
			this->size = size;
			return *this;
		}

		BufferBuilder& Usage(BufferUsage usage)
		{
			this->usage = usage;
			return *this;
		}

		BufferBuilder& Target(BufferTarget target)
		{
			this->target = target;
			return *this;
		}

		friend Buffer::Buffer(GLuint handle, size_t size, BufferUsage usage, BufferTarget target);
		friend class Buffer;
		std::shared_ptr<Buffer> Create()
		{
			Validate();

			glGenBuffers(1, &handle);
			glBindBuffer(static_cast<GLenum>(target), handle);
			glBufferData(static_cast<GLenum>(target), size, data, static_cast<GLenum>(usage));
			Logger::Info("Created buffer ", handle);
			
			auto buffer = std::make_shared<Buffer>(handle, size, usage, target);
			glManager.buffers[buffer->GetHandle()] = buffer;
			return buffer;
		}

		void Validate()
		{
			if (usage == BufferUsage::None
				|| target == BufferTarget::None)
			{
				std::abort();
			}
		}
	private:
		GLuint handle;
		void* data;
		size_t size;
		BufferUsage usage = BufferUsage::None;
		BufferTarget target = BufferTarget::None;
	};
	class VertexArrayBuilder
	{
	public:
		VertexArrayBuilder& IBO(std::shared_ptr<Buffer> ibo)
		{
			this->ibo = ibo;
			return *this;
		}
		VertexArrayBuilder& VBO(std::shared_ptr<Buffer> vbo)
		{
			this->vbo = vbo;
			return *this;
		}
		VertexArrayBuilder& Attributes(const std::vector<int>& attrLens)
		{
			this->attrLens = attrLens;
			return *this;
		}
		void Validate()
		{
			if (vbo == nullptr
				|| attrLens.size() == 0)
			{
				std::abort();
			}
		}
		std::shared_ptr<VertexArray> Create()
		{
			Logger::Info("Creating vertex array ", handle, ".");
			Validate();
			glGenVertexArrays(1, &handle);
			SetAttribs();
			auto vao = std::make_shared<VertexArray>(handle, attrLens, stride, vbo, ibo);
			glManager.vaos[vao->GetHandle()] = vao;
			return vao;
		}
	private:
		void SetAttribs()
		{
			this->attrLens = attrLens;
			glBindVertexArray(handle);
			vbo->Bind(BufferTarget::Vertex);
			if (ibo != nullptr)
			{
				ibo->Bind(BufferTarget::Index);
			}

			stride = 0;
			for (int i = 0; i < attrLens.size(); i++)
				stride += attrLens[i];
			stride *= sizeof(float);

			int offset = 0;
			for (int i = 0; i < attrLens.size(); i++)
			{
				glVertexAttribPointer(i, attrLens[i], GL_FLOAT, false, stride, (void*)(offset * sizeof(float)));
				glEnableVertexAttribArray(i);
				offset += attrLens[i];
			}
		}
	private:
		std::vector<int> attrLens;
		int stride;
		GLuint handle;
		std::shared_ptr<Buffer> vbo, ibo;
	};
	class TextureBuilder
	{
	public:
		TextureBuilder& Params(TextureParams params)
		{
			this->params = params;
			return *this;
		}
		TextureBuilder& Attributes(TextureAttributes attributes)
		{
			this->attributes = attributes;
			return *this;
		}
		std::shared_ptr<Texture2D> Create2DWithData(void* data)
		{
			data2D = data;
			return Create2D();
		}
		std::shared_ptr<TextureCube> CreateCubeWithData(std::array<void*, 6> data)
		{
			dataCube = data;
			return CreateCube();
		}
		std::shared_ptr<Texture2D> Create2DEmpty()
		{
			data2D = nullptr;
			return Create2D();
		}
		std::shared_ptr<TextureCube> CreateCubeEmpty()
		{
			dataCube = { nullptr };
			return CreateCube();
		}
	private:
		std::shared_ptr<Texture2D> Create2D()
		{
			Create(false);
			auto tex = std::make_shared<Texture2D>(handle, params, attributes);
			Logger::Info("Created texture 2D ", handle, ".");
			glManager.texture2Ds[tex->GetHandle()] = tex;
			return tex;
		}
		std::shared_ptr<TextureCube> CreateCube()
		{
			Create(true);
			auto tex = std::make_shared<TextureCube>(handle, params, attributes);
			Logger::Info("Created texture Cube ", handle, ".");
			glManager.textureCubes[tex->GetHandle()] = tex;
			return tex;
		}
		void Create(bool cube)
		{
			Validate();
			glGenTextures(1, &handle);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, handle);
			SetParams(cube);
			SetData(cube);
		}
		void Validate()
		{
			if (attributes.width == 0 || attributes.height == 0 || attributes.format == TextureFormat::None
				|| params.filter == TextureFilter::None || params.wrap == TextureWrap::None)
			{
				std::abort();
			}
		}
		void SetData(bool cube)
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
		void SetParams(bool cube)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.GetFilter());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.magLinear ? GL_LINEAR : GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.GetWrap());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.GetWrap());
			if (cube)
			{
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, params.GetWrap());
			}
		}
	private:
		GLuint handle;
		std::array<void*, 6> dataCube;
		void* data2D = nullptr;
		TextureParams params;
		TextureAttributes attributes;
	};
	class FramebufferBuilder
	{
	public:
		std::shared_ptr<Framebuffer> Create()
		{
			Validate();
			glGenFramebuffers(1, &handle);
			SetAttachments();
			auto fb = std::make_shared<Framebuffer>(handle, width, height, attachments);
			glManager.fbos[fb->GetHandle()] = fb;
			Logger::Info("Created framebuffer ", handle);
			return fb;
		}
		FramebufferBuilder& Attachments(FramebufferAttachments attachements)
		{
			this->attachments = attachements;
			if (attachments.depth != nullptr)
			{
				this->width = attachments.depth->GetWidth();
				this->height = attachments.depth->GetHeight();
			}
			else
			{
				this->width = attachments.colors[0].first->GetWidth();
				this->height = attachments.colors[0].first->GetHeight();
			}
			return *this;
		}
	protected:
		void SetAttachments()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, handle);
			if (attachments.depth != nullptr)
			{
				AddDepthTexture(attachments.depth->GetHandle());
			}
			for (auto att : attachments.colors)
			{
				if (att.first->GetWidth() != width
					|| att.first->GetHeight() != height)
				{
					Logger::Error("A framebuffer's attachments should be of the same resolution");
				}
				AddColorTexture(att.first->GetHandle(), att.second);
			}
			UpdateDrawBuffers();
		}
		void UpdateDrawBuffers()
		{
			if (attachments.colors.size() > 0)
			{
				auto buf = std::vector<GLenum>(attachments.colors.size());
				for (size_t i = 0; i < buf.size(); i++)
					buf[i] = GL_COLOR_ATTACHMENT0 + attachments.colors[i].second;
				glDrawBuffers(buf.size(), buf.data());
			}
			else
			{
				glDrawBuffer(GL_NONE);
			}
		}
		void AddColorTexture(GLuint texture, int idx)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, texture, 0);
		}
		void AddDepthTexture(GLuint texture)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
		}
		void Validate()
		{
			if (width <= 0 || height <= 0 
				|| (attachments.colors.size() == 0 && attachments.depth == nullptr))
			{
				std::abort();
			}

			std::unordered_set<int> colorIdxs;
			for (auto& color : attachments.colors)
			{
				if (colorIdxs.contains(color.second))
				{
					MetaLogger().Error(color.second, " color idx is already set. This is probably not what you want.");
				}
				else
				{
					colorIdxs.insert(color.second);
				}
			}

		}
	protected:
		GLuint handle;
		int width = 0, height = 0;
		FramebufferAttachments attachments;
	};
	class GeometryFramebufferBuilder : FramebufferBuilder
	{
	public:
		GeometryFramebufferBuilder& Width(int width)
		{
			this->width = width;
			return *this;
		}
		GeometryFramebufferBuilder& Height(int height)
		{
			this->height = height;
			return *this;
		}
		GeometryFramebufferBuilder& Depth(TextureFormat depth)
		{
			this->depth = depth;
			return *this;
		}
		GeometryFramebufferBuilder& Position(TextureFormat position)
		{
			this->position = position;
			return *this;
		}
		GeometryFramebufferBuilder& Normal(TextureFormat normal)
		{
			this->normal = normal;
			return *this;
		}
		GeometryFramebufferBuilder& Diffuse(TextureFormat diffuse)
		{
			this->diffuse = diffuse;
			return *this;
		}
		GeometryFramebufferBuilder& Specular(TextureFormat specular)
		{
			this->specular = specular;
			return *this;
		}
		std::shared_ptr<GeometryFramebuffer> Create()
		{
			Validate();
			CreateAttachments();
			FramebufferBuilder::Validate();
			glGenFramebuffers(1, &handle);
			SetAttachments();

			auto fb = std::make_shared<GeometryFramebuffer>(handle, width, height, attachments);
			glManager.fbos[fb->GetHandle()] = fb;
			Logger::Info("Created geometry framebuffer ", handle);
			return fb;
		}
	private:
		void Validate()
		{
			if (depth == TextureFormat::None || position == TextureFormat::None || diffuse == TextureFormat::None || 
				normal == TextureFormat::None || specular == TextureFormat::None)
			{
				std::abort();
			}
		}
		void CreateAttachments()
		{
			auto createTexture = [&](TextureFormat format)
			{
				return TextureBuilder()
					.Params(TextureParams::FramebufferTex2DParams())
					.Attributes(TextureAttributes{ .width = (uint32_t)width, .height = (uint32_t)height, .format = format })
					.Create2DEmpty();
			};
			attachments = FramebufferAttachments()
				.Color(createTexture(position), 0)
				.Color(createTexture(diffuse), 1)
				.Color(createTexture(normal), 2)
				.Color(createTexture(specular), 3)
				.Depth(createTexture(depth));
		}
	private:
		TextureFormat 
			depth = TextureFormat::None,
			position = TextureFormat::None, 
			diffuse = TextureFormat::None,
			normal = TextureFormat::None, 
			specular = TextureFormat::None;
	};
	class LightFramebufferBuilder : FramebufferBuilder
	{
	public:
		LightFramebufferBuilder& Width(int width)
		{
			this->width = width;
			return *this;
		}
		LightFramebufferBuilder& Height(int height)
		{
			this->height = height;
			return *this;
		}
		LightFramebufferBuilder& ColorFormat(TextureFormat color)
		{
			this->color = color;
			return *this;
		}
		LightFramebufferBuilder& DepthTexture(std::shared_ptr<Texture2D> depth)
		{
			this->depth = depth;
			return *this;
		}
		std::shared_ptr<LightFramebuffer> Create()
		{
			Validate();
			width = depth->GetWidth();
			height = depth->GetHeight();
			CreateAttachments();
			FramebufferBuilder::Validate();
			glGenFramebuffers(1, &handle);
			SetAttachments();

			auto fb = std::make_shared<LightFramebuffer>(handle, width, height, attachments);
			glManager.fbos[fb->GetHandle()] = fb;
			Logger::Info("Created geometry framebuffer ", handle);
			return fb;
		}
	private:
		void Validate()
		{
			if (depth == nullptr || color == TextureFormat::None)
			{
				std::abort();
			}
		}
		void CreateAttachments()
		{
			attachments = FramebufferAttachments()
				.Color(TextureBuilder()
					.Params(TextureParams::FramebufferTex2DParams())
					.Attributes(TextureAttributes{ .width = (uint32_t)width, .height = (uint32_t)height, .format = color})
					.Create2DEmpty(), 0)
				.Depth(depth);
		}
	private:
		std::shared_ptr<Texture2D> depth = nullptr;
		TextureFormat color = TextureFormat::None;
	};
	class ShaderBuilder
	{
	public:
		ShaderBuilder& Sources(const std::unordered_map<ShaderType, std::string>& sources)
		{
			this->sources = sources;
			return *this;
		}
		std::shared_ptr<Shader> Create()
		{
			handle = glCreateProgram();
			Compile();
			Logger::Info("Created shader ", handle);
			auto shader = std::make_shared<Shader>(handle);
			glManager.shaders[shader->GetHandle()] = shader;
			return shader;
		}
	private:
		void Compile()
		{
			std::unordered_map<GLenum, int> handles;

			int success;
			char infoLog[512] = {};

			for (auto& s : sources)
			{
				GLenum type = std::to_underlying(s.first);
				int handle = glCreateShader(type);
				const GLchar* src = s.second.c_str();
				glShaderSource(handle, 1, &src, nullptr);
				glCompileShader(handle);

				glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
				if (!success)
				{
					Logger::Error("SHADER::COMPILATION_FAILED");
					glGetShaderInfoLog(handle, 512, NULL, infoLog);
					Logger::Error(infoLog);
				}
				handles[type] = handle;
			}

			for (auto& s : handles)
			{
				glAttachShader(handle, s.second);
			}

			glLinkProgram(handle);

			glGetProgramiv(handle, GL_LINK_STATUS, &success);
			if (!success)
			{
				Logger::Error("SHADER::LINKING_FAILED");
				glGetProgramInfoLog(handle, 512, NULL, infoLog);
				Logger::Error(infoLog);
			}

			for (auto& s : handles)
			{
				glDeleteShader(s.second);
			}
		}
	private:
		GLuint handle; 
		std::unordered_map<ShaderType, std::string> sources;
	};
}	
