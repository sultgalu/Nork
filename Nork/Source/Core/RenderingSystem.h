#pragma once

#include "Scene/Scene.h"
#include "Modules/Renderer/Objects/Shader/Shader.h"
#include "Modules/Renderer/Pipeline/Light/DirShadowMap.h"
#include "Modules/Renderer/Pipeline/Light/PointShadowMap.h"
#include "Modules/Renderer/Config.h"
#include "Modules/Renderer/Model/DrawBatch.h"
#include "Modules/Renderer/Storage/DrawState.h"
#include "Modules/Renderer/Pipeline/Pipeline.h"
#include "Modules/Renderer/Pipeline/Stages/DeferredStage.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"

namespace Nork {
	class Shaders
	{
		bool RecompileShader(std::shared_ptr<Renderer::Shader>&, const std::string& src);
		std::shared_ptr<Renderer::Shader>& FindShader(std::shared_ptr<Renderer::Shader>);
	public:
		std::shared_ptr<Renderer::Shader> InitShaderFromSource(const std::string& path);
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
		float pointInternalSize = 0.8f;
		float pointAA = 0.33f;
		float pointAlpha = 1.0f;
		int pointSize = 20;
		float pointBias = 0.26f;
		glm::vec4 pointColor = glm::vec4(1, 0, 0, 1.0f);

		float lineWidth = 0.005f;
		float lineAlpha = 1.0f;
		float dummy; // for alignment

		glm::vec4 lineColor = glm::vec4(0, 0, 1, 0.3f);
		glm::vec4 triColor = glm::vec4(0, 1, 0, 0.2f);

		glm::vec3 selectedColor = glm::vec3(1, 0, 1);
	};
	struct SceneView
	{
		SceneView(uint32_t width, uint32_t height, Renderer::TextureFormat format = Renderer::TextureFormat::RGBA16F)
		{
			camera = std::make_shared<Components::Camera>();
			pipeline = std::make_shared<Renderer::Pipeline>(width, height, format);
			CreateFb();
		}
		SceneView& operator=(const SceneView& other)
		{
			camera = other.camera;
			pipeline = other.pipeline;
			CreateFb();
		}
		SceneView(const SceneView& other)
		{
			*this = other;
		}
		std::shared_ptr<Components::Camera> camera;
		std::shared_ptr<Renderer::Pipeline> pipeline;
		std::shared_ptr<Renderer::Framebuffer> fb;
	private:
		void CreateFb();
	};
	class RenderingSystem : public Renderer::DrawCommandProvider
	{
	public:
		RenderingSystem(entt::registry& registry);
		void BeginFrame();
		void Update();
		void EndFrame();
		GlobalShaderUniform& GetGlobalShaderUniform() { return globalShaderUniform; }
		void DrawToScreen(int w, int h);
		void SetRenderColliders(bool);
		template<std::derived_from<Renderer::Stage> T> std::shared_ptr<T> CreateStage(Renderer::Pipeline& dest)
		{
			if (auto stage = dest.Get<T>())
			{ // reuse stages (their textures/framebuffers)
				return stage;
			}
			for (auto& sceneView : sceneViews)
			{ // reuse stages from pipelines with same resoution
				auto& pipeline = sceneView->pipeline;
				if (pipeline.get() != &dest && pipeline->FinalTexture()->GetWidth() == dest.FinalTexture()->GetWidth() && pipeline->FinalTexture()->GetHeight() == dest.FinalTexture()->GetHeight())
				{
					if (auto stage = pipeline->Get<T>())
					{
						return stage;
					}
				}
			}
			return ConstructStage<T>(dest);
		}
		const std::vector<Renderer::DrawCommandMultiIndirect>& operator()() override
		{
			static std::vector<Renderer::DrawCommandMultiIndirect> commands;
			commands = { drawBatch.GetDrawCommand() };
			return commands;
		}
	private:
		void UpdateGlobalUniform();
		template<std::derived_from<Renderer::Stage> T> std::shared_ptr<T> ConstructStage(Renderer::Pipeline& dest);
		void RenderColliders();
		void CreateCollidersVao(size_t count);
		void UpdateLights();
		void ViewProjectionUpdate(Components::Camera& camera);
		void DrawBatchUpdate();

	private:
		glm::uvec2 resolution = { 1920, 1080 };
		entt::observer dirLightObserver;
		entt::observer pointLightObserver;
	public:
		entt::registry& registry;
		Shaders shaders;
		Renderer::DrawState drawState;
		Renderer::DrawBatch drawBatch;
		Observed<GlobalShaderUniform> globalShaderUniform;
		std::shared_ptr<Renderer::VertexArray> colliderVao = nullptr;
		std::unordered_set<std::shared_ptr<SceneView>> sceneViews;
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