#pragma once

#include "../../View.h"
#include "Modules/Renderer/Vulkan/Image.h"
#include "Editor/Utils/EditorImage.h"

namespace Nork::Editor {
	class ViewportView: public View
	{
	public:
		ViewportView();
		void Content() override;
		struct MouseState
		{
			int mousePosX = 0, mousePosY = 0;
			bool isViewportHovered = false;
			bool isViewportDoubleClicked = false;
			bool isViewportClicked = false;
		};
	public:
		EditorImage image;
		MouseState mouseState;
		// std::shared_ptr<SceneView> sceneView;
		std::shared_ptr<Components::Camera> camera;
		std::shared_ptr<CameraController> camController;
	};
}