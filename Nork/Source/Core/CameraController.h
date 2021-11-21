#pragma once

#include "Core/InputState.h"
#include "Components/Camera.h"

namespace Nork
{
	class CameraController
	{
	public:
		CameraController(Receiver& receiver, Components::Camera& cam)
			: camera(cam)
		{
			SetupInputHandling(receiver);
		}
		void Update(float delta);
	private:
		void SetupInputHandling(Receiver& receiver);
		void HandleScroll(const MouseScrollEvent& event);
		void HandleMouseMove(const MouseMoveEvent& event);
	public:
		Components::Camera& camera;
	};
}