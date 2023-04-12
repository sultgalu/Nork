#pragma once
#include "Core/Engine.h"

namespace Nork::Editor {

	struct CommonData
	{
		std::shared_ptr<SceneNode> selectedNode = nullptr;
		bool gameMode = false;
		bool sceneRunning = false;
	};
	extern CommonData& _GetCommonData();
	extern Engine& _GetEngine();

	class View
	{
	public:
		virtual ~View() {}
		virtual void Content() = 0;
	protected:
		Engine& GetEngine() { return Engine::Get(); }
		Renderer::Vulkan::Window& GetWindow() { return Renderer::Vulkan::Window::Instance(); }
		const Input& GetInput() { return Input::Instance(); }
		Scene& GetScene() { return GetEngine().scene; }
		PhysicsSystem& GetPhysics() { return GetEngine().physicsSystem; }
		RenderingSystem& GetRenderer() { return RenderingSystem::Instance(); }
		CommonData& GetCommonData() { return _GetCommonData(); }
	private:
	};
}