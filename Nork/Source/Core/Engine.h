#pragma once
#include "NorkWindow.h"
#include "Core/CameraController.h"
#include "Scene/Scene.h"
#include "PolygonMesh.h"
#include "Modules/Physics/Data/World.h"
#include "Modules/Physics/Pipeline/PhysicsSystem.h"
#include "Modules/Renderer2/Objects/Shader/Shader.h"
#include "RenderingSystem.h"

namespace Nork
{
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
		static std::unordered_map<Renderer::ShaderType, std::string> SplitShaderContent(std::string source);
	private:
		void PhysicsUpdate();
		void OnDShadowAdded(entt::registry& reg, entt::entity ent);
		void OnDShadowRemoved(entt::registry& reg, entt::entity ent);
	public:
		Scene::Scene scene;
		GLuint idMap;

		RenderingSystem renderingSystem;
		bool drawPolies = false, drawLines = true, drawPoints = true, drawTriangles = true, drawSky = false;
		bool satRes = false, gjkRes = false, clipRes = false, aabbRes = false;
		bool sat = false, gjk = false, clip = true, aabb = true;
		bool physicsUpdate = false;
		bool updatePoliesForPhysics = true;
		std::optional<std::pair<glm::vec3, std::pair<uint8_t, glm::vec3>>> collisionRes;
		float physicsSpeed = 1.0f;

		Physics::System pSystem;
		Physics::World& pWorld = pSystem.world;

		static std::vector<std::pair<std::string, float>> GetDeltas();
	};

	extern Engine& GetEngine();
}