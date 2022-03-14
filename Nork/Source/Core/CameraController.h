#pragma once

#include "Core/InputState.h"
#include "Components/Camera.h"
#include "Scene/Scene.h"

namespace Nork
{
	class CameraController
	{
	public:
		CameraController(Receiver& receiver, Scene& scene)
			: scene(scene)
		{
			SetupInputHandling(receiver);
		}
		void Update(float delta);
	private:
		void SetupInputHandling(Receiver& receiver);
		void HandleScroll(const MouseScrollEvent& event);
		void HandleMouseMove(const MouseMoveEvent& event);
	public:
		Scene& scene;
	};
}