#pragma once

#include "Core/InputState.h"
#include "Components/Camera.h"

namespace Nork
{
	class CameraController
	{
	public:
		CameraController(Event::Receiver& receiver, Components::Camera& cam)
			: camera(cam)
		{
			SetupInputHandling(receiver);
		}
		void Update(float delta);
	private:
		void SetupInputHandling(Event::Receiver& receiver);
		void HandleScroll(const Event::Types::MouseScroll& event);
		void HandleMouseMove(const Event::Types::MouseMove& event);
	public:
		Components::Camera& camera;
	};
}