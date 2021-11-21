#pragma once

#include "../Data/Ligths.h"
#include "../Data/Mesh.h"
#include "../Data/Shader.h"
#include "Framebuffer.h"
#include "Buffers.h"

namespace Nork::Renderer
{
	using SFB = Framebuffer<Utils::Texture::Format::Depth16>;
	class DirShadowFramebuffer : private SFB
	{
	public:
		using SFB::Framebuffer;
		using SFB::ClearAndUse;
		
		GLuint Texture()
		{
			return depth;
		}
	};

	class PointShadowFramebuffer : private SFB
	{
	public:
		PointShadowFramebuffer(uint32_t width, uint32_t height)
			: Framebuffer(width, height, CreateTexture(width, height))
		{
		}
		using SFB::ClearAndUse;

		GLuint Texture()
		{
			return depth;
		}
	private:
		GLuint CreateTexture(uint32_t width, uint32_t height)
		{
			using namespace Utils::Texture;
			auto tex = Create<TextureType::Cube>(width, height, Format::Depth16, nullptr, TextureParams::CubeMapParams());
			return tex;
		}
	};

	struct UBOLightData
	{
		uint32_t dLightCount = 0, dShadowCount = 0,
			pLightCount = 0, pShadowCount = 0;
	};

	struct LightCullConfig
	{
		glm::mat4 V;
		glm::mat4 iP;
		uint32_t lightCount;
		uint32_t atomicCounter;
		glm::uvec2 cullRes;
	};

	class LightManager
	{
	public:
		uint32_t cullQ = 5;
	public:
		LightManager(Shader);
		void Update();
		void DrawPointShadowMap(const PointLight& light, const PointShadow& shadow, const std::span<Model> models, PointShadowFramebuffer& fb, Shader& pShadowMapShader);
		void DrawDirShadowMap(const DirLight& light, const DirShadow& shadow, const std::span<Model> models, DirShadowFramebuffer& fb, Shader& dShadowMapShader);
		
		void SetPointLightData(std::span<PointLight> pls, std::span<PointShadow> pss);
		void SetPointLightData(std::span<PointLight> pls, std::span<PointShadow> pss, glm::mat4 view, glm::mat4 proj);
		void SetDirLightData(std::span<DirLight> dls, std::span<DirShadow> dss);
		GLuint GetDebug();
		void SetDebug(Shader shader);
		void PointLightCulling(std::span<PointLight> lights, glm::mat4 view, glm::mat4 proj);
	private:
		UBO<PointLight> pointLightUBO;
		UBO<PointShadow> pointShadowUBO;
		UBO<DirLight> dirLightUBO; 
		UBO<DirShadow> dirShadowUBO;
		UBO<UBOLightData> commonUBO;
		UBOLightData commonData = UBOLightData();

		SSBO<glm::uvec2> pLightRangesSSBO;
		SSBO<uint32_t> pLightIndicesSSBO;
		SSBO<LightCullConfig> configSSBO;

		Shader lightCullShader;
	};
}

