#pragma once
#include "NorkWindow.h"
#include "Modules/Renderer/Pipeline/Deferred.h"
#include "Modules/Renderer/Pipeline/LightManager.h"
#include "Core/CameraController.h"
#include "Scene/Scene.h"
#include "PolygonMesh.h"
#include "Modules/Physics/Data/World.h"
#include "Modules/Physics/Pipeline/PhysicsSystem.h"

namespace Nork
{
	using namespace Renderer::Pipeline;

	struct EngineConfig
	{
		EngineConfig() = default;

		uint32_t width = 1280, height = 720;
		inline EngineConfig& SetResolution(uint32_t w, uint32_t h) { width = w; height = h; return *this; }
	};

	class Engine: Template::Types::OnlyConstruct
	{
	public:
		Engine(EngineConfig config);
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
		bool updatePoliesForPhysics = true;
		std::optional<std::pair<glm::vec3, std::pair<uint8_t, glm::vec3>>> collisionRes;
		float physicsSpeed = 1.0f;

		Physics::System pSystem;
		Physics::World& pWorld = pSystem.world;

		Event::Dispatcher dispatcher;

		static std::vector<std::pair<std::string, float>> GetDeltas();
	};

	extern std::optional<Components::Camera*> GetActiveCamera();
	extern Engine& GetEngine();
}