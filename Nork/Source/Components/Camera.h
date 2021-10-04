#pragma once

#include "Modules/ECS/Storage.h"

namespace Nork::Components
{
	struct Camera
	{
		enum struct Direction
		{
			None = 0,
			Up,
			Down,
			Left,
			Rigth,
			Forward,
			Backward,
		};

		Camera() : Camera(45.0f, 1920.f / 1080.f, 0.1f, 1000.0f) {}
		Camera(float fieldOfViewDegrees, float aspectRatio, float nearClip, float farClip);
		// float GetYaw() { return yaw; }
		// glm::vec3 GetFront() { return front; }
		// glm::vec3 GetPosition() { return position; }
		// glm::mat4 GetView() { return view; }
		// glm::mat4 GetProjection() { return projection; }
		// float GetSpeed() { return moveSpeed; }
		void Move(Direction direction, float delta);
		void Zoom(float delta);
		void Rotate(float deltaPitch, float deltaYaw);
		void SetRotation(float pitch, float yaw);
		void SetPosition(glm::vec3 pos);
	public:
		//inline const glm::mat4& GetViewProjection() const { return this->viewProjection; } 
	public:
		float FOV, ratio, nearClip, farClip;
		float pitch = 0.0f, yaw = 0.0f, moveSpeed = 0.01f, zoomSpeed = 0.1f, rotationSpeed = 0.1f;
		glm::vec3 position, front, up, right;
		glm::mat4 view, projection, viewProjection;
	private:
		void UpdateFrontRight();
		inline void UpdateView() { this->view = glm::lookAt(this->position, this->position + this->front, this->up); }
		inline void UpdateProjection() { this->projection = glm::perspective(this->FOV, this->ratio, this->nearClip, this->farClip); }
		inline void UpdateViewProjection() { this->viewProjection = this->projection * this->view; }
	};
}