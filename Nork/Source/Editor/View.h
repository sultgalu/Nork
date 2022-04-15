#pragma once
#include "Core/Engine.h"

namespace Nork::Editor {

	struct CommonData
	{
		std::shared_ptr<SceneNode> selectedNode = nullptr;
		std::vector<std::shared_ptr<Components::Camera>> editorCameras;
	};
	extern CommonData& _GetCommonData();
	extern Engine& _GetEngine();

	class View
	{
	public:
		virtual ~View() {}
		virtual void Content() = 0;
	protected:
		Engine& GetEngine() { return _GetEngine(); }
		Window& GetWindow() { return GetEngine().window; }
		const Input& GetInput() { return GetEngine().window.Input(); }
		Scene& GetScene() { return GetEngine().scene; }
		PhysicsSystem& GetPhysics() { return GetEngine().physicsSystem; }
		RenderingSystem& GetRenderer() { return GetEngine().renderingSystem; }
		CommonData& GetCommonData() { return _GetCommonData(); }
	private:
	};
}