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

		std::counting_semaphore<10> downloadSem;
		std::counting_semaphore<10> uploadSem;
		std::counting_semaphore<10> phxSem;
		bool waitingForUpload = false;
		bool phxCalc = false;
		bool phxCalcDone = false;
		bool downloading = false;
		bool uploading = false;
	private:
		std::vector<Components::Camera> cameras;
		void OnDrawableAdded(entt::registry& reg, entt::entity id);
		std::thread* LaunchPhysicsThread();
	};
}