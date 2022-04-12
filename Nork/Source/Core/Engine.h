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
#include "ScriptSystem.h"

namespace Nork
{
	class Engine
	{
	public:
		Engine();
		Engine(Engine&&) = delete;
		void Update();
		void StartPhysics();
		void StopPhysics();
		void AddCamera(Components::Camera cam);
		auto& Cameras() { return cameras; }
	public:
		Scene scene;

		RenderingSystem renderingSystem = RenderingSystem(scene.registry);
		ResourceManager resourceManager = ResourceManager(renderingSystem.drawState);
		
		PhysicsSystem physicsSystem;
		std::thread* physicsThread;
		bool physicsUpdate = false;
		std::mutex physicsDownloadLock;
		std::mutex physicsUploadLock;
		std::mutex physicsLock;

		bool scriptUpdated = false;
		std::binary_semaphore updateSem;
		std::binary_semaphore uploadSem;

		ScriptSystem scriptSystem;
	private:
		std::vector<Components::Camera> cameras;
		void OnDrawableAdded(entt::registry& reg, entt::entity id);
		std::thread* LaunchPhysicsThread();
	};
}