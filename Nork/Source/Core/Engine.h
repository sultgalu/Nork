#pragma once
#include "Platform/Window.h"
#include "Modules/Renderer/Pipeline/Deferred.h"
#include "Modules/Renderer/Pipeline/LightManager.h"
#include "Core/CameraController.h"
#include "Scene/Scene.h"
#include "MeshWorld.h"
#include "Modules/Physics/Data/World.h"
#include "Modules/Physics/Pipeline/PhysicsSystem.h"

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
		void UpdatePoliesForPhysics();
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
		
		//MeshWorld<Vertex> meshes = MeshWorld<Vertex>::GetCube();
		
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

		bool drawPolies = false, drawLines = true, drawPoints = true, drawTriangles = true, drawSky = false;
		bool satRes = false, gjkRes = false, clipRes = false, aabbRes = false;
		bool sat = false, gjk = false, clip = true, aabb = true;
		bool physicsUpdate = false;
		float targetDelta;
		std::optional<std::pair<glm::vec3, std::pair<uint8_t, glm::vec3>>> collisionRes;

		Physics::World pWorld;
		Physics::System pSystem;
	};

	extern std::optional<Components::Camera*> GetActiveCamera();
	extern Engine& GetEngine();

}