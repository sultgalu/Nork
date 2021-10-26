#pragma once
#include "Platform/Window.h"
#include "Modules/Renderer/Pipeline/Deferred.h"
#include "Modules/Renderer/Pipeline/LightManager.h"
#include "Core/CameraController.h"
#include "Scene/Scene.h"
#include "MeshWorld.h"

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
		void ReadId(int x, int y);
	private:
		DeferredData CreatePipelineResources();
		void SyncComponents();
		void UpdateLights();
		void PhysicsUpdate();
		void DrawHitboxes();
		void ViewProjectionUpdate();
		void OnDShadowAdded(entt::registry& reg, entt::entity ent);
		void OnDShadowRemoved(entt::registry& reg, entt::entity ent);
	public:
		Window window;
		Deferred pipeline;
		LightManager lightMan;
		Event::Dispatcher appEventMan;
		Scene::Scene scene;
		struct Vertex
		{
			glm::vec3 pos;
			float selected = 0;
			uint32_t id;
			inline static uint32_t idCounter = 0;
			Vertex(glm::vec3 pos, bool selected = false)
			{
				this->pos = pos;
				this->selected = selected;
				this->id = ++idCounter;
			}
		};
		//MeshWorld<Vertex> meshes = MeshWorld<Vertex>::GetCube();
		std::vector<MeshWorld<Vertex>> colliders;
		
		std::vector<Renderer::Pipeline::DirShadowFramebuffer> dShadowFramebuffers;
		std::vector<Renderer::Pipeline::PointShadowFramebuffer> pShadowFramebuffers;
		Renderer::Pipeline::GeometryFramebuffer geometryFb;
		Renderer::Pipeline::LightPassFramebuffer lightFb;
		GLuint idMap;

		int pointSize = 20;
		float pointInternalSize = 0.5f, pointAA = 0.3f, lineWidth = 0.005f;

		float pointAlpha = 1.0f, lineAlpha = 1.0f, triAlpha = 0.6f;
		glm::vec4 pointColor = { 1,0,0, 1.0f};
		glm::vec4 lineColor = { 0,0,1, 0.3f};
		glm::vec4 triangleColor = { 0,1,0, 0.4f};
		glm::vec3 selectedColor = { 1,0,1 };

		bool drawLines = true, drawPoints = true, drawTriangles = true, drawSky = false;
		bool faceQ = false;
	};

	extern std::optional<Components::Camera*> GetActiveCamera();
	extern Engine& GetEngine();

}