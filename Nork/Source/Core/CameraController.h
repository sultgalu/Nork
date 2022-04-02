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
		void Update(Components::Camera&, float delta);
	};
}