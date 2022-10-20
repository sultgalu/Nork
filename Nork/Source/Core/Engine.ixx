export module Nork.Core:Engine;

export import :NorkWindow;
export import :CameraController;
export import :ResourceManager;
export import :RenderingSystem;
export import :PhysicsSystem;
export import :ScriptSystem;

export namespace Nork
{
	class Engine
	{
	public:
		Engine();
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
		std::thread* LaunchPhysicsThread();
		void UpdateGlobalTransforms();
		void UpdateTransformMatrices();
	};
}