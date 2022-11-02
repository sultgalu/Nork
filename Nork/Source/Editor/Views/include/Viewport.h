#pragma once

#include "../../View.h"

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
		MouseState mouseState;
		std::shared_ptr<SceneView> sceneView;
		std::shared_ptr<CameraController> camController = std::make_shared<FpsCameraController>();
	};
}