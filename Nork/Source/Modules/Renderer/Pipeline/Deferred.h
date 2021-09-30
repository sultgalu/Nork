#pragma once

#include "../Data/Shader.h"
#include "../Data/Texture.h"
#include "../Data/Mesh.h"
#include "../Data/Ligths.h"

namespace Nork::Renderer::Data
{
	typedef std::pair<std::vector<Mesh>, glm::mat4> Model;
}

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

		DeferredData(Shaders shaders) : shaders(shaders) {}

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
		void DrawScene(std::span<Model> models, std::span<DirLight> dLights, std::span<PointLight> pLights);
	private:
		void DrawGBuffers(std::span<Model> models);
		void DrawLightPass(std::span<DirLight> dLights, std::span<PointLight> pLights);
		void DrawSkybox();
	public:
		DeferredData data;
	};
}

