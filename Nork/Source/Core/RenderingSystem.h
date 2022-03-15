#pragma once

#include "Scene/Scene.h"
#include "Modules/Renderer/Objects/GLManager.h"
#include "Modules/Renderer/Objects/Shader/Shader.h"
#include "Modules/Renderer/Model/Model.h"
#include "Modules/Renderer/Pipeline/LightStateSynchronizer.h"
#include "Modules/Renderer/Pipeline/DirShadowMap.h"
#include "Modules/Renderer/Pipeline/PointShadowMap.h"
#include "Modules/Renderer/Pipeline/Deferred/DeferredPipeline.h"
#include "Components/All.h"

namespace Nork {
	
	class Shaders
	{
	private:
		static std::string GetFileContent(std::string path)
		{
			std::ifstream ifs(path);
			std::stringstream ss;
			ss << ifs.rdbuf();
			return ss.str();
		}
		static Renderer::ShaderType GetTypeByString(std::string_view str)
		{
			using enum Renderer::ShaderType;
			if (str._Equal("vertex")) [[likely]]
				return Vertex;
			else if (str._Equal("fragment"))
				return Fragment;
			else if (str._Equal("geometry"))
				return Geometry;
			else if (str._Equal("compute"))
				return Compute;

			Logger::Error("ERR:: Unkown shader type ", str);
			std::abort(); // :(
		}
		static std::unordered_map<Renderer::ShaderType, std::string> SplitShaders(std::string content)
		{
			const char* label = "#type";
			size_t labelLen = strlen(label);

			std::unordered_map<Renderer::ShaderType, std::string> shaderSrcs;

			size_t pos = content.find(label, 0);
			Renderer::ShaderType shadType;
			do
			{ // the file must start with #type
				size_t start = pos;
				size_t eol = content.find_first_of("\r\n", pos); // idx of the end of the "#type" line

				pos += labelLen + 1; // the first idx of "vertex"
				std::string type = content.substr(pos, eol - pos); // must be "#type vertex" with whitespaces exactly like that.
				shadType = GetTypeByString(type);

				//size_t nextLinePos = s.find_first_not_of("\r\n", eol); // filtering out spaces
				pos = content.find(label, eol); // next idx starting with #type
				if (pos == std::string::npos) // no more shaders
				{
					shaderSrcs[shadType] = content.substr(eol);
					break;
				}
				shaderSrcs[shadType] = content.substr(eol, pos - eol); // more shaders ahead

			} while (true);
			return shaderSrcs;
		}
		static std::shared_ptr<Renderer::Shader> InitShaderFromSource(std::string path)
		{
			return Renderer::ShaderBuilder().Sources(SplitShaders(GetFileContent(path))).Create();
		}
	public:
		Shaders()
		{
			SetLightPassShader(InitShaderFromSource("Source/Shaders/lightPass.shader"));
			SetGeometryPassShader(InitShaderFromSource("Source/Shaders/gPass.shader"));
			dShadowShader = InitShaderFromSource("Source/Shaders/dirShadMap.shader");
			pShadowShader = InitShaderFromSource("Source/Shaders/pointShadMap.shader");
			skyboxShader = InitShaderFromSource("Source/Shaders/skybox.shader");
			pointShader = InitShaderFromSource("Source/Shaders/point.shader");
			lineShader = InitShaderFromSource("Source/Shaders/line.shader");
			textureShader = InitShaderFromSource("Source/Shaders/texture.shader");
		}
		void SetLightPassShader(std::shared_ptr<Renderer::Shader> shader)
		{
			lPassShader = shader;
			lPassShader->Use()
				.SetInt("gPos", 0)
				.SetInt("gDiff", 1)
				.SetInt("gNorm", 2)
				.SetInt("gSpec", 3);

			for (int i = 0; i < 5; i++)
				lPassShader->SetInt("dirShadowMaps[" + std::to_string(i) + "]", i + 10);
			for (int i = 0; i < 5; i++)
				lPassShader->SetInt("pointShadowMaps[" + std::to_string(i) + "]", i + 15);
		}
		void SetGeometryPassShader(std::shared_ptr<Renderer::Shader> shader)
		{
			gPassShader = shader;
			using enum Renderer::TextureMapType;
			gPassShader->Use()
				.SetInt("materialTex.diffuse", (int)Diffuse)
				.SetInt("materialTex.normals", (int)Normal)
				.SetInt("materialTex.roughness", (int)Roughness)
				.SetInt("materialTex.reflect", (int)Reflection);
		}
		std::shared_ptr<Renderer::Shader> gPassShader, lPassShader,
			dShadowShader, pShadowShader,
			skyboxShader, textureShader,
			pointShader, lineShader;
	};

	class RenderingSystem
	{
	public:
		RenderingSystem();
		void UpdateLights(entt::registry& reg);
		void ViewProjectionUpdate(Components::Camera& camera);
		void SyncComponents(entt::registry& reg);
		void RenderScene(entt::registry& reg);
		void Update(Scene& scene);
	public:
		int pointSize = 20;
		float pointInternalSize = 0.5f, pointAA = 0.3f, lineWidth = 0.005f;
		float pointAlpha = 1.0f, lineAlpha = 1.0f, triAlpha = 0.6f;
		glm::vec4 pointColor = { 1,0,0, 1.0f };
		glm::vec4 lineColor = { 0,0,1, 0.3f };
		glm::vec4 triangleColor = { 0,1,0, 0.4f };
		glm::vec3 selectedColor = { 1,0,1 };
		glm::uvec2 resolution = { 1920, 1080 };
	public:
		Shaders shaders;
		Renderer::DeferredPipeline deferredPipeline;
		Renderer::LightStateSynchronizer lightStateSyncher;
		std::array<std::shared_ptr<Renderer::DirShadowMap>, 5> dirShadowMaps;
		std::array<std::shared_ptr<Renderer::PointShadowMap>, 5> pointShadowMaps;
	};
}