#include "CameraController.h"
#include "App/Application.h"

namespace Nork
{
FpsCameraController::FpsCameraController()
	: moveSpeed(0.01f), zoomSpeed(0.1f), rotationSpeed(0.1f)
{}
void FpsCameraController::UpdateByKeyInput(Components::Camera& camera, float delta)
{
	using enum Components::Camera::Direction;
	auto& input = Input::Instance();

	if (input.IsJustPressed(Key::Space))
	{
		moveSpeed *= 10;
	}
	if (input.IsJustReleased(Key::Space))
	{
		moveSpeed /= 10.f;
	}
	delta *= moveSpeed * camera.FOV;

	if (input.IsDown(Key::W))
	{
		auto dir = input.IsDown(Key::Shift) ? Up : Forward;
		camera.Move(dir, delta);
	}
	if (input.IsDown(Key::S))
	{
		auto dir = input.IsDown(Key::Shift) ? Down : Backward;
		camera.Move(dir, delta);
	}
	if (input.IsDown(Key::A))
	{
		camera.Move(Left, delta);
	}
	if (input.IsDown(Key::D))
	{
		camera.Move(Rigth, delta);
	}
}
void FpsCameraController::UpdateByMouseInput(Components::Camera& camera)
{
	auto& input = Input::Instance();

	if (input.DidScroll())
	{
		camera.Zoom(input.ScrollOffs() * zoomSpeed * camera.FOV);
	}

	if (input.IsDown(Button::Left))
	{
		float speed = rotationSpeed * camera.FOV;
		camera.Rotate(input.CursorYOffs() * speed, -input.CursorXOffs() * speed);
	}
}

EditorCameraController::EditorCameraController(const glm::vec3& center)
	: center(center), zoomSpeed(0.1f), rotationSpeed(0.1f)
{}
void EditorCameraController::UpdateByKeyInput(Components::Camera& camera, float delta)
{
	auto& input = Input::Instance();
}
void EditorCameraController::UpdateByMouseInput(Components::Camera& camera)
{
	auto& input = Input::Instance();

	camera.FOV = 1.0f;
	auto vec = center - camera.position;
	auto distance = glm::length(vec);
	auto dir = (center - camera.position) / distance;
	auto diff = glm::dot(dir, camera.front);
	if (diff < 0.999f)
	{
		auto pitch = glm::degrees(glm::asin(dir.y));
		auto yaw = glm::degrees(std::atan2(-dir.z, dir.x));
		// auto a = dir.x / glm::cos(glm::radians(camera.pitch));
		// auto yaw = glm::degrees(glm::acos(a));
		camera.SetRotation(pitch, yaw);
		camera.SetPosition(center + -camera.front * distance);
	}
	if (input.DidScroll())
	{
		if (input.ScrollOffs() < 0 || distance > 0.1f) {
			camera.Move(Components::Camera::Direction::Forward, input.ScrollOffs() * distance * zoomSpeed);
		}
	}

	if (input.IsDown(Button::Left))
	{
		float yOffs = -input.CursorYOffs() * (1 + std::abs(input.CursorYOffs()) / 10.0) * rotationSpeed;
		float xOffs = input.CursorXOffs() * (1 + std::abs(input.CursorXOffs()) / 10.0) * rotationSpeed;
		constexpr float max = 100;
		if (std::abs(yOffs) > max)
			yOffs = max * glm::sign(yOffs);
		if (std::abs(xOffs) > max)
			xOffs = max * glm::sign(xOffs);
		camera.Rotate(yOffs, xOffs);
		camera.SetPosition(center + -camera.front * distance);
	}
}
void EditorCameraController::SetCenter(Components::Camera& camera, const glm::vec3& newCenter) {
	auto vec = center - camera.position;
	camera.SetPosition(newCenter - vec);
	center = newCenter;
}
void EditorCameraController::SetDistance(Components::Camera& camera, float newDistance) {
	auto direction = glm::normalize(center - camera.position);
	camera.SetPosition(center - direction * newDistance);
}
}

