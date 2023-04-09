#pragma once

#include "Core/InputState.h"
#include "Components/Camera.h"
#include "Scene/Scene.h"

namespace Nork
{
	class CameraController
	{
	public:
		virtual void UpdateByKeyInput(Components::Camera&, float delta) = 0;
		virtual void UpdateByMouseInput(Components::Camera&) = 0;
	};
	class FpsCameraController : public CameraController
	{
	public:
		FpsCameraController();
		void UpdateByKeyInput(Components::Camera&, float delta) override;
		void UpdateByMouseInput(Components::Camera&) override;
		float moveSpeed, zoomSpeed, rotationSpeed;
	};
	class EditorCameraController : public CameraController
	{
	public:
		EditorCameraController(const glm::vec3& center);
		void UpdateByKeyInput(Components::Camera&, float delta) override;
		void UpdateByMouseInput(Components::Camera&) override;
		void SetCenter(Components::Camera&, const glm::vec3& center);
		void SetDistance(Components::Camera&, float distance);
	public:
		glm::vec3 center;
		float zoomSpeed, rotationSpeed;
	};
}