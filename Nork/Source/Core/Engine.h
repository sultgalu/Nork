#pragma once
#include "Platform/Window.h"
#include "Modules/Renderer/Pipeline/Deferred.h"
#include "Modules/Renderer/Pipeline/LightManager.h"
#include "Core/CameraController.h"
#include "Scene/Scene.h"

namespace Nork
{
	using namespace Renderer;
	using namespace Pipeline;

	struct EngineConfig
	{
		EngineConfig() = default;

		uint32_t width = 1280, height = 720;
		inline EngineConfig& SetResolution(uint32_t w, uint32_t h) { width = w; height = h; return *this; }
	};

	class Engine
	{
	public:
		Engine(EngineConfig& config);
		~Engine();
		void Launch();
	private:
		DeferredData CreatePipelineResources();
		void SyncComponents();
		void UpdateLights();
		void ViewProjectionUpdate();
		void OnDShadowAdded(entt::registry& reg, entt::entity ent);
		void OnDShadowRemoved(entt::registry& reg, entt::entity ent);
	public:
		Window window;
		Deferred pipeline;
		LightManager lightMan;
		Event::Dispatcher appEventMan;
		Scene::Scene scene;
		
		std::vector<Renderer::Pipeline::DirShadowFramebuffer> dShadowFramebuffers;
		std::vector<Renderer::Pipeline::PointShadowFramebuffer> pShadowFramebuffers;
		Renderer::Pipeline::GeometryFramebuffer geometryFb;
		Renderer::Pipeline::LightPassFramebuffer lightFb;
	};

	extern std::optional<Components::Camera*> GetActiveCamera();
	extern Engine& GetEngine();

}