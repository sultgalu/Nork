#pragma once

#include "Scene/Scene.h"
#include "Modules/Renderer/Objects/Shader/Shader.h"
#include "Modules/Renderer/Pipeline/Light/LightState.h"
#include "Modules/Renderer/Pipeline/Light/DirShadowMap.h"
#include "Modules/Renderer/Pipeline/Light/PointShadowMap.h"
#include "Modules/Renderer/Pipeline/Deferred/DeferredPipeline.h"
#include "Modules/Renderer/Config.h"
#include "Modules/Renderer/Model/DrawBatch.h"

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
		static std::shared_ptr<Renderer::Shader> InitShaderFromSource(std::string path);
	public:
		Shaders();
		void SetLightPassShader(std::shared_ptr<Renderer::Shader> shader);
		void SetGeometryPassShader(std::shared_ptr<Renderer::Shader> shader);
		std::shared_ptr<Renderer::Shader> gPassShader, lPassShader,
			dShadowShader, pShadowShader,
			skyboxShader, textureShader,
			pointShader, lineShader;
	};
	struct GlobalShaderUniform
	{
		float pointInternalSize = 0.5f;
		float pointAA = 0.3f;
		float pointAlpha = 1.0f;
		int pointSize = 20;
		glm::vec4 pointColor = glm::vec4(1, 0, 0, 1.0f);

		float lineWidth = 0.005f;
		float lineAlpha = 1.0f;
		float triAlpha = 0.6f;
		float dummy; // for alignment

		glm::vec4 lineColor = glm::vec4(0, 0, 1, 0.3f);

		glm::vec3 selectedColor = glm::vec3(1, 0, 1);
	};
	class RenderingSystem
	{
	public:
		RenderingSystem(std::shared_ptr<Renderer::VertexArray>);

		void UpdateGlobalUniform();
		void UpdateLights(entt::registry& reg);
		void ViewProjectionUpdate(Components::Camera& camera);
		void SyncComponents(entt::registry& reg);
		void RenderScene(entt::registry& reg);
		void Update(entt::registry& registry, Components::Camera& camera);
		void DrawBatchUpdate(entt::registry& reg);

		GlobalShaderUniform GetGlobalShaderUniform() { return globalShaderUniform; }
	
	private:
		glm::uvec2 resolution = { 1920, 1080 };
	public:
		Shaders shaders;
		Renderer::DeferredPipeline deferredPipeline;
		Renderer::LightState lightState;
		std::array<std::shared_ptr<Renderer::DirShadowMap>, Renderer::Config::LightData::dirShadowsLimit> dirShadowMaps;
		std::array<std::shared_ptr<Renderer::PointShadowMap>, Renderer::Config::LightData::pointShadowsLimit> pointShadowMaps;
		std::shared_ptr<Renderer::TextureCube> skybox;
		Renderer::DrawBatch drawBatch;

		Observed<GlobalShaderUniform> globalShaderUniform;
		bool drawSky = true;
	};
}