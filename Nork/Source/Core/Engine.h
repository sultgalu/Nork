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
	class Engine
	{
	public:
		Engine();
		Engine(Engine&&) = delete;
		void Launch();
	public:
		Scene scene;

		RenderingSystem renderingSystem = RenderingSystem(scene.registry);
		ResourceManager resourceManager = ResourceManager(renderingSystem.drawState);
		
		PhysicsSystem physicsSystem;
		bool physicsUpdate = false;
	private:
		void OnDrawableAdded(entt::registry& reg, entt::entity id);
	};
}