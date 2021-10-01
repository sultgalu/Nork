#include "pch.h"
#include "../Camera.h"

namespace Nork::Components
{
	Camera::Camera(float FieldOfViewDegrees, float aspectRatio, float nearClip, float farClip)
	{
		this->FOV = glm::radians(FieldOfViewDegrees);
		this->ratio = aspectRatio;
		this->nearClip = nearClip;
		this->farClip = farClip;
		this->UpdateProjection();

		this->front = glm::vec3(0.0f, 0.0f, -1.0f);
		this->up = glm::vec3(0.0f, 1.0f, 0.0f);
		this->position = glm::vec3(0.0f, 0.0f, 0.0f);
		this->UpdateView();

		this->UpdateViewProjection();
	}

	void Camera::UpdateView()
	{
		this->view = glm::lookAt(this->position, this->position + this->front, this->up);
	}
	void Camera::UpdateProjection()
	{
		this->projection = glm::perspective<float>(this->FOV, this->ratio, this->nearClip, this->farClip);
	}

	void Camera::UpdateViewProjection()
	{
		this->viewProjection = this->projection * this->view;
	}

	void Camera::Move(MoveDirection direction, float delta)
	{
		if ((direction & MoveDirection::Up) != MoveDirection::None)
		{
			this->position += glm::normalize(this->up) * this->moveSpeed * delta;
		}
		if ((direction & MoveDirection::Down) != MoveDirection::None)
		{
			this->position -= glm::normalize(this->up) * this->moveSpeed * delta;
		}
		if ((direction & MoveDirection::Left) != MoveDirection::None)
		{
			this->position -= GetRight() * this->moveSpeed * delta;
		}
		if ((direction & MoveDirection::Rigth) != MoveDirection::None)
		{
			this->position += GetRight() * this->moveSpeed * delta;
		}
		if ((direction & MoveDirection::Forward) != MoveDirection::None)
		{
			this->position += this->front * this->moveSpeed * delta;
		}
		if ((direction & MoveDirection::Backward) != MoveDirection::None)
		{
			this->position -= this->front * this->moveSpeed * delta;
		}
		this->UpdateView();
		this->UpdateViewProjection();
	}

	void Camera::Zoom(float delta)
	{
		float diff = delta * this->zoomSpeed;

		static const float maxFOV = glm::radians(120.0f);
		static const float minFOV = glm::radians(1.0f);
		if (this->FOV - diff < minFOV)
			diff = this->FOV - minFOV;
		if (this->FOV - diff > maxFOV)
			diff = this->FOV - maxFOV;

		this->zoomSpeed -= diff / this->FOV * zoomSpeed;
		this->moveSpeed -= diff / this->FOV * moveSpeed;
		this->rotationSpeed -= diff / this->FOV * rotationSpeed;
		this->FOV -= diff;

		this->UpdateProjection();
		this->UpdateViewProjection();
	}

	void Camera::SetRotation(float pitch, float yaw)
	{
		this->pitch = pitch;

		if (this->pitch > 89)
		{
			this->pitch = 89;
		}
		else if (this->pitch < -89)
		{
			this->pitch = -89;
		}

		this->yaw = yaw;

		this->UpdateFront();
		this->UpdateView();
		this->UpdateViewProjection();
	}

	void Camera::Rotate(float deltaPitch, float deltaYaw)
	{
		this->pitch += (deltaPitch * rotationSpeed);
		if (this->pitch > 89)
		{
			this->pitch = 89;
		}
		else if (this->pitch < -89)
		{
			this->pitch = -89;
		}
		this->yaw += (deltaYaw * rotationSpeed);

		this->UpdateFront();
		this->UpdateView();
		this->UpdateViewProjection();
	}

	void Camera::UpdateFront()
	{
		glm::vec3 newDirection;
		newDirection.x = glm::cos(glm::radians(this->yaw)) * glm::cos(glm::radians(this->pitch));
		newDirection.y = glm::sin(glm::radians(this->pitch));
		newDirection.z = glm::sin(glm::radians(this->yaw)) * glm::cos(glm::radians(this->pitch));
		this->front = glm::normalize(newDirection);
	}
}