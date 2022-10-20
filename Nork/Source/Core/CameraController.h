#pragma once

#include "Core/InputState.h"
import Nork.Components;
import Nork.Scene;

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