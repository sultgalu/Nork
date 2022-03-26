#pragma once
#include "NorkWindow.h"
#include "Core/CameraController.h"
#include "Scene/Scene.h"
#include "PolygonMesh.h"
#include "Modules/Physics/Data/World.h"
#include "Modules/Renderer/Objects/Shader/Shader.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"
#include "Core/ResourceManager.h"

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
	private:
		void OnDShadowAdded(entt::registry& reg, entt::entity ent);
		void OnDShadowRemoved(entt::registry& reg, entt::entity ent);
		void OnDrawableAdded(entt::registry& reg, entt::entity id);
	public:
		Scene scene;
		GLuint idMap;

		ResourceManager resourceManager;
		RenderingSystem renderingSystem = RenderingSystem(resourceManager.vaoWrapper.GetVertexArray());
		PhysicsSystem physicsSystem;
		bool physicsUpdate = false;
	};

	extern Engine& GetEngine();
}