#pragma once

#include "Modules/ECS/Storage.h"

namespace Nork::Components
{
	struct Camera
	{
		enum struct MoveDirection
		{
			None = 0,
			Up = 1 << 0,
			Down = 1 << 1,
			Left = 1 << 2,
			Rigth = 1 << 3,
			Forward = 1 << 4,
			Backward = 1 << 5,
		};

		Camera() : Camera(45.0f, (float)1920 / (float)1080, 0.1f, 1000.0f) {}
		Camera(float fieldOfViewDegrees, float aspectRatio, float nearClip, float farClip);
		glm::mat4 GetViewProjection() { return this->viewProjection; }
		float GetYaw() { return yaw; }
		glm::vec3 GetFront() { return front; }
		inline glm::vec3 GetRight() { return glm::cross(front, up); }
		glm::vec3 GetPosition() { return position; }
		glm::mat4 GetView() { return view; }
		glm::mat4 GetProjection() { return projection; }
		float GetSpeed() { return moveSpeed; }
		void Move(MoveDirection direction, float delta);
		void Zoom(float delta);
		void Rotate(float deltaPitch, float deltaYaw);
		void SetPosition(glm::vec3 pos) { position = pos; this->UpdateView(); this->UpdateViewProjection(); }
		void SetRotation(float pitch, float yaw);

		float FOV, ratio, nearClip, farClip;
		float pitch = 0.0f, yaw = 0.0f, moveSpeed = 0.01f, zoomSpeed = 0.1f, rotationSpeed = 0.1f;
		glm::vec3 position, front, up;
		glm::mat4 view, projection, viewProjection;
	private:
		void UpdateFront();
		void UpdateView();
		void UpdateProjection();
		void UpdateViewProjection();
	};

	inline Camera::MoveDirection operator|(Camera::MoveDirection dir1, Camera::MoveDirection dir2)
	{
		return static_cast<Camera::MoveDirection>(static_cast<int>(dir1) | static_cast<int>(dir2));
	}
	inline Camera::MoveDirection operator&(Camera::MoveDirection dir1, Camera::MoveDirection dir2)
	{
		return static_cast<Camera::MoveDirection>(static_cast<int>(dir1) & static_cast<int>(dir2));
	}
}