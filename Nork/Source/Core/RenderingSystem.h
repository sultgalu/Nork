#pragma once

#include "Scene/Scene.h"
#include "Modules/Renderer/Objects/Shader/Shader.h"
#include "Modules/Renderer/Pipeline/Light/DirShadowMap.h"
#include "Modules/Renderer/Pipeline/Light/PointShadowMap.h"
#include "Modules/Renderer/Pipeline/Deferred/DeferredPipeline.h"
#include "Modules/Renderer/Config.h"
#include "Modules/Renderer/Model/DrawBatch.h"
#include "Modules/Renderer/Storage/DrawState.h"
#include "Modules/Renderer/Pipeline/PostProcess/Bloom.h"

namespace Nork {

	struct Viewport
	{
		enum Source
		{
			Deferred = 1 << 0,
			Bloom = 1 << 5,
			Tonemap = 1 << 9,
			Sky = 1 << 10,
			Colliders = 1 << 15,

			PostProcess = Bloom | Tonemap,
			Default = Deferred | Tonemap | Sky,
			Debug = Colliders
		};

		Viewport(std::shared_ptr<Components::Camera> camera);
		std::shared_ptr<Renderer::Framebuffer> fb;
		std::shared_ptr<Components::Camera> camera = nullptr;
		std::shared_ptr<Renderer::Texture> target = nullptr;
		std::shared_ptr<Renderer::Texture> Texture() { return fb->GetAttachments().colors[0].first; }
		Source source = Source::Default;
		bool active = true;
		bool Renders(Source s) { return (source & s) == s; }
	};

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
		std::shared_ptr<Renderer::Shader> InitShaderFromSource(const std::string& path);
		bool RecompileShader(std::shared_ptr<Renderer::Shader>&, const std::string& src);
		std::shared_ptr<Renderer::Shader>& FindShader(std::shared_ptr<Renderer::Shader>);
	public:
		std::shared_ptr<Renderer::Shader> RecompileShader(std::shared_ptr<Renderer::Shader>);
		Shaders();
		void SetLightPassShader(std::shared_ptr<Renderer::Shader> shader);
		std::shared_ptr<Renderer::Shader> gPassShader, lPassShader,
			dShadowShader, pShadowShader,
			skyboxShader, skyShader, textureShader,
			pointShader, lineShader, colliderShader,
			bloomShader, bloom2Shader, bloom3Shader, 
			tonemapShader;
		std::vector<std::pair<std::shared_ptr<Renderer::Shader>, std::string>> shaderSources;
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
		float dummy; // for alignment

		glm::vec4 lineColor = glm::vec4(0, 0, 1, 0.3f);
		glm::vec4 triColor = glm::vec4(0, 1, 0, 0.2f);

		glm::vec3 selectedColor = glm::vec3(1, 0, 1);
	};
	class RenderingSystem
	{
	public:
		RenderingSystem(entt::registry& registry);
		void BeginFrame();
		void Update();
		void Update(Viewport&);
		void EndFrame();
		GlobalShaderUniform GetGlobalShaderUniform() { return globalShaderUniform; }
		void DrawToScreen(int w, int h);
		void SetRenderColliders(bool);
	private:
		void RenderColliders();
		void CreateCollidersVao(size_t count);
		void UpdateGlobalUniform();
		void UpdateLights();
		void ViewProjectionUpdate(Components::Camera& camera);
		void RenderScene(Viewport& viewport);
		void DrawBatchUpdate();

	private:
		glm::uvec2 resolution = { 1920, 1080 };
		entt::observer dirLightObserver;
		entt::observer pointLightObserver;
	public:
		entt::registry& registry;
		Shaders shaders;
		Renderer::DeferredPipeline deferredPipeline;
		std::shared_ptr<Renderer::TextureCube> skybox;
		Renderer::DrawState drawState;
		Renderer::DrawBatch drawBatch;
		Observed<GlobalShaderUniform> globalShaderUniform;
		std::shared_ptr<Renderer::VertexArray> colliderVao = nullptr;
		Renderer::Bloom bloom;
		std::shared_ptr<Renderer::Framebuffer> target;
		std::vector<std::shared_ptr<Viewport>> viewports;
		float delta;
	private:
		void OnDShadAdded(entt::registry& reg, entt::entity id);
		void OnPShadAdded(entt::registry& reg, entt::entity id);
		void OnDShadRemoved(entt::registry& reg, entt::entity id);
		void OnPShadRemoved(entt::registry& reg, entt::entity id);

		void OnDLightAdded(entt::registry& reg, entt::entity id);
		void OnPLightAdded(entt::registry& reg, entt::entity id);
		void OnDLightRemoved(entt::registry& reg, entt::entity id);
		void OnPLightRemoved(entt::registry& reg, entt::entity id);
	};
}