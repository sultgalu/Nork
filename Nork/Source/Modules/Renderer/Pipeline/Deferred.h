#pragma once

#include "../Data/Shader.h"
#include "../Data/Texture.h"
#include "../Data/Mesh.h"
#include "../Data/Ligths.h"
#include "Framebuffer.h"

using namespace Nork::Renderer::Data;

namespace Nork::Renderer::Pipeline
{
	using GFB = Framebuffer<TextureFormat::Depth16, TextureFormat::RGB16F, TextureFormat::RGB16F, TextureFormat::RGB16F, TextureFormat::RGBA16F>;
	using LFB = Framebuffer<TextureFormat::Depth16, TextureFormat::RGBA16F>;

	class GeometryFramebuffer : private GFB
	{
	public:
		using GFB::GFB;
		using GFB::Use;
		using GFB::Clear;
		using GFB::ClearAndUse;
		using GFB::Width;
		using GFB::Height;

		inline GLuint Depth()
		{
			return depth;
		}
		inline GLuint Position()
		{
			return colors[0];
		}
		inline GLuint Diffuse()
		{
			return colors[1];
		}
		inline GLuint Normal()
		{
			return colors[2];
		}
		inline GLuint Specular()
		{
			return colors[3];
		}
	};
	class LightPassFramebuffer : private LFB
	{
	public:
		LightPassFramebuffer(GLuint depth, uint32_t width, uint32_t height)
			: LFB(std::unordered_map<TextureFormat, GLuint> { { TextureFormat::Depth16, depth } }, width, height)
		{
		}

		using LFB::Use;
		using LFB::Width;
		using LFB::Height;
		using LFB::Clear;
		using LFB::ClearAndUse;

		inline GLuint Result()
		{
			return colors[0];
		}
		inline GLuint Depth()
		{
			return depth;
		}
		inline void Clear()
		{
			Clear(GL_COLOR_BUFFER_BIT);
		}
		inline void ClearAndUse()
		{
			ClearAndUse(GL_COLOR_BUFFER_BIT);
		}
	};

	struct DeferredData
	{
		struct Shaders
		{
			Shader gPass;
			Shader lPass;
			Shader skybox;
		};

		DeferredData(Shaders shaders, GLuint skyboxTex)
		{
			SetGPassShader(shaders.gPass);
			SetLPassShader(shaders.lPass);
			this->shaders.skybox = shaders.skybox;
			this->skyboxTex = skyboxTex;
		}

		int mainResX, mainResY;

		void SetGPassShader(Shader shader);
		void SetLPassShader(Shader shader);

		Shaders shaders;

		GLuint skyboxTex;
	};

	class Deferred
	{
	public:
		Deferred(DeferredData);
		~Deferred() = default;
		void DrawScene(std::span<Model> models, LightPassFramebuffer& lightFb, GeometryFramebuffer& gFb);
		void UseShadowMap(DirShadow shadow, ShadowFramebuffer& fb);
		void UseShadowMap(PointShadow shadow, ShadowFramebuffer& fb);
	private:
		void DrawGBuffers(std::span<Model> models);
		void DrawLightPass(GeometryFramebuffer& gFb);
		void DrawSkybox();
	public:
		DeferredData data;
	};
}

