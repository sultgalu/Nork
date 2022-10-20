export module Nork.Editor.Views:View;

import Nork.Core;
import Nork.Scene;
import Nork.Components;

export namespace Nork::Editor {

	struct CommonData
	{
		std::shared_ptr<SceneNode> selectedNode = nullptr;
		std::vector<std::shared_ptr<Components::Camera>> editorCameras;
		bool gameMode = false;
	};

	class View
	{
	public:
		virtual ~View() {}
		virtual void Content() = 0;
	protected:
		Engine& GetEngine();
		CommonData& GetCommonData();
		Window& GetWindow() { return GetEngine().window; }
		const Input& GetInput() { return GetEngine().window.Input(); }
		Scene& GetScene() { return GetEngine().scene; }
		PhysicsSystem& GetPhysics() { return GetEngine().physicsSystem; }
		RenderingSystem& GetRenderer() { return GetEngine().renderingSystem; }
	private:
	};
}