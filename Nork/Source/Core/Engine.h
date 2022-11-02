#pragma once
#include "NorkWindow.h"
#include "Core/CameraController.h"
#include "Scene/Scene.h"
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
		static Engine& Get();
		Engine(Engine&&) = delete;
		void Update();
		void StartPhysics(bool startScript = true);
		void StopPhysics();
	public:
		Nork::Window window;
		Scene scene;

		RenderingSystem renderingSystem = RenderingSystem(scene.registry);
		ResourceManager resourceManager = ResourceManager(renderingSystem.drawState);
		
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

		entt::observer transformObserver;
	private:
		void OnDrawableAdded(entt::registry& reg, entt::entity id);
		void OnPhysicsAdded(entt::registry& reg, entt::entity id);
		void OnPhysicsRemoved(entt::registry& reg, entt::entity id);
		std::thread* LaunchPhysicsThread();
		void UpdateGlobalTransforms();
		void UpdateTransformMatrices();
	};

	class CollidersStage : public Renderer::Stage
	{
	public:
		CollidersStage(Scene& scene, Shaders& shaders);
		bool Execute(Renderer::Framebuffer& src, Renderer::Framebuffer& dst) override;
	public:
		Scene& scene;
		Shaders& shaders;
		std::shared_ptr<Renderer::VertexArray> vao;
		std::shared_ptr<Renderer::Framebuffer> fb;
	};
}