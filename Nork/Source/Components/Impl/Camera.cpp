#include "pch.h"
#include "../Camera.h"

namespace Nork::Components
{
Camera::Camera()
	:
	Camera(45.0f, 1920.f / 1080.f, 0.1f, 1000.0f)
{}
Camera::Camera(float FieldOfViewDegrees, float aspectRatio, float nearClip, float farClip)
{
	this->FOV = glm::radians(FieldOfViewDegrees);
	this->ratio = aspectRatio;
	this->nearClip = nearClip;
	this->farClip = farClip;
	this->UpdateProjection();

	this->up = glm::vec3(0.0f, 1.0f, 0.0f);
	this->position = glm::vec3(0.0f, 0.0f, 0.0f);

	this->UpdateFrontRight();
	this->UpdateView();

	this->UpdateViewProjection();
}

void Camera::Move(Direction direction, float delta)
{
	using enum Direction;
	switch (direction)
	{
		case Up:
			this->position += glm::normalize(this->up) * delta;
			break;
		case Down:
			this->position -= glm::normalize(this->up) * delta;
			break;
		case Left:
			this->position -= right * delta;
			break;
		case Rigth:
			this->position += right * delta;
			break;
		case Forward:
			this->position += this->front * delta;
			break;
		case Backward:
			this->position -= this->front * delta;
			break;
		case None: [[unlikely]]
			break;
	}

	this->UpdateView();
	this->UpdateViewProjection();
}

void Camera::Zoom(float delta)
{
	float diff = delta;

	static const float maxFOV = glm::radians(120.0f);
	static const float minFOV = glm::radians(1.0f);
	if (this->FOV - diff < minFOV)
		diff = this->FOV - minFOV;
	if (this->FOV - diff > maxFOV)
		diff = this->FOV - maxFOV;

	this->FOV -= diff;

	this->UpdateProjection();
	this->UpdateViewProjection();
}

void Camera::SetPosition(glm::vec3 pos)
{
	position = pos;
	this->UpdateView();
	this->UpdateViewProjection();
}

void Camera::Update()
{
	UpdateFrontRight();
	UpdateView();
	UpdateProjection();
	UpdateViewProjection();
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

	this->UpdateFrontRight();
	this->UpdateView();
	this->UpdateViewProjection();
}

void Camera::Rotate(float deltaPitch, float deltaYaw)
{
	if (deltaPitch == 0 && deltaYaw == 0)
		return;
	this->pitch += deltaPitch;
	if (this->pitch > 89)
	{
		this->pitch = 89;
	}
	else if (this->pitch < -89)
	{
		this->pitch = -89;
	}
	this->yaw += deltaYaw;

	this->UpdateFrontRight();
	this->UpdateView();
	this->UpdateViewProjection();
}

void Camera::UpdateFrontRight()
{
	glm::vec3 newDirection;
	newDirection.x = glm::cos(glm::radians(this->yaw)) * glm::cos(glm::radians(this->pitch));
	newDirection.y = glm::sin(glm::radians(this->pitch));
	newDirection.z = glm::sin(glm::radians(this->yaw)) * glm::cos(glm::radians(this->pitch));
	this->front = glm::normalize(newDirection);
	this->right = glm::cross(front, up);
}
void Camera::UpdateView() 
{ 
	view = glm::lookAt(position, position + front, up); 
}
void Camera::UpdateProjection() 
{ 
	projection = glm::perspective(FOV, ratio, nearClip, farClip); 
	projection[1][1] *= -1; // flip for VULKAN Screen Space
}
void Camera::UpdateViewProjection() 
{
	viewProjection = projection * view; 
}

}