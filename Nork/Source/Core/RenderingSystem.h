#pragma once

#include "Scene/Scene.h"
#include "Modules/Renderer/Objects/Shader/Shader.h"
#include "Modules/Renderer/Pipeline/Pipeline.h"
#include "Modules/Renderer/Pipeline/Stages/DeferredStage.h"
#include "Modules/Renderer/Pipeline/DrawCommands/DrawObjectsCommand.h"
#include "Modules/Renderer/Pipeline/Stages/ShadowMapStage.h"
#include "Core/ResourceManager.h"

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
		}
		SceneView& operator=(const SceneView& other)
		{
			camera = other.camera;
			pipeline = other.pipeline;
		}
		SceneView(const SceneView& other)
		{
			*this = other;
		}
		std::shared_ptr<Components::Camera> camera;
		std::shared_ptr<Renderer::Pipeline> pipeline;
	};
	class RenderingSystem
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
	private:
		void UpdateGlobalUniform();
		template<std::derived_from<Renderer::Stage> T> std::shared_ptr<T> ConstructStage(Renderer::Pipeline& dest);
		void UpdateLights();
		void ViewProjectionUpdate(Components::Camera& camera);

	private:
		glm::uvec2 resolution = { 1920, 1080 };
		entt::observer dirLightObserver;
		entt::observer pointLightObserver;
	public:
		struct ShadowMapProvider : Renderer::ShadowMapProvider
		{
			std::span<Renderer::PointShadowMap> PointShadowMaps() override { return pShadMaps; }
			std::span<Renderer::DirShadowMap> DirShadowMaps() override { return dShadMaps; }
			std::vector<Renderer::PointShadowMap> pShadMaps;
			std::vector<Renderer::DirShadowMap> dShadMaps;
		};
		entt::registry& registry;
		Shaders shaders;
		Renderer::World world;
		ResourceManager resourceManager = ResourceManager(world);
		Renderer::DrawObjectsCommand deferredDrawCommand;
		Renderer::DrawObjectsCommand shadowMapDrawCommand;
		ShadowMapProvider shadowMapProvider;
		Observed<GlobalShaderUniform> globalShaderUniform;
		std::shared_ptr<Renderer::VertexArray> colliderVao = nullptr;
		std::unordered_set<std::shared_ptr<SceneView>> sceneViews;
		float delta;
	private:
		bool shouldUpdateDrawCommands = false;
		void UpdateDrawCommands();
		void OnDrawableAdded(entt::registry& reg, entt::entity id);
		void OnDrawableUpdated(entt::registry& reg, entt::entity id);

		bool shouldUpdateDirLightAndShadows = false;
		bool shouldUpdatePointLightAndShadows = false;
		void UpdateDirLightShadows(); // updates UBO idxs and shadowMapProvider
		void UpdatePointLightShadows();

		void OnDShadRemoved(entt::registry& reg, entt::entity id);
		void OnPShadRemoved(entt::registry& reg, entt::entity id);
		void OnDLightRemoved(entt::registry& reg, entt::entity id);
		void OnPLightRemoved(entt::registry& reg, entt::entity id);

		void OnDLightAdded(entt::registry& reg, entt::entity id);
		void OnPLightAdded(entt::registry& reg, entt::entity id);
		void OnDShadAdded(entt::registry& reg, entt::entity id);
		void OnPShadAdded(entt::registry& reg, entt::entity id);
	};
}