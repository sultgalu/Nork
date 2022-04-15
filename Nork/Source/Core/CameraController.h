#pragma once

#include "Core/InputState.h"
#include "Components/Camera.h"
#include "Scene/Scene.h"

namespace Nork
{
	class CameraController
	{
	public:
		CameraController()
		{}
		static void UpdateByKeyInput(Components::Camera&, float delta);
		static void UpdateByMouseInput(Components::Camera&, float delta);
	};
}