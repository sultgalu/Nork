#pragma once

#include "../Data/Shader.h"
#include "../Data/Texture.h"
#include "../Data/Mesh.h"
#include "../Data/Ligths.h"
#include "Framebuffer.h"

using namespace Nork::Renderer::Data;

namespace Nork::Renderer::Pipeline
{
	struct DeferredData
	{
		struct GBufferData
		{
			GLuint fb, pos, diff, norm, depth;
		};

		struct LightPassData
		{
			GLuint  fb, tex;
		};

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

		GBufferData gBuffer = GBufferData{};
		LightPassData lightPass = LightPassData{};

		int mainResX, mainResY;

		void SetMainRes(int x, int y);
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
		void DrawScene(std::span<Model> models);
		void UseShadowMap(DirShadow shadow, ShadowFramebuffer& fb);
		void UseShadowMap(PointShadow shadow, ShadowFramebuffer& fb);
	private:
		void DrawGBuffers(std::span<Model> models);
		void DrawLightPass();
		void DrawSkybox();
	public:
		DeferredData data;
	};
}

