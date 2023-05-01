#pragma once
#include "Core/CameraController.h"
#include "Scene/Scene.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"
#include "../Scripting/ScriptSystem.h"
#include "Modules/Renderer/Vulkan/Window.h"

namespace Nork
{
	class Engine
	{
	public:
		Engine();
		static Engine& Get();
		Engine(Engine&&) = delete;
		void Update();
		void StartPhysics(bool startScript = true);
		void StopPhysics();
	public:
		Scene scene;

		RenderingSystem renderingSystem;
		PhysicsSystem physicsSystem;
		std::thread* physicsThread;
		bool physicsUpdate = false;
		std::mutex physicsDownloadLock;
		std::mutex physicsUploadLock;
		std::mutex physicsLock;

		bool scriptUpdate = false;
		bool scriptUpdated = false;
		std::binary_semaphore updateSem;
		std::binary_semaphore uploadSem;

		ScriptSystem scriptSystem;
	private:
		void OnPhysicsAdded(entt::registry& reg, entt::entity id);
		void OnPhysicsRemoved(entt::registry& reg, entt::entity id);
		std::thread* LaunchPhysicsThread();
		void UpdateGlobalTransforms();
		void UpdateTransformMatrices();
	};
}