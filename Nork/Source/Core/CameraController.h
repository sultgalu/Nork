#pragma once

#include "Input.h"
#include "Components/Camera.h"

namespace Nork
{
	class CameraController
	{
	public:
		CameraController(Event::Receiver& receiver, Input::State& input, Components::Camera& cam)
			: camera(cam), input(input)
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
	private:
		Input::State& input;
	};
}