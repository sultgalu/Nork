#include "CameraController.h"
#include "App/Application.h"

namespace Nork
{
	void FpsCameraController::UpdateByKeyInput(Components::Camera& camera, float delta)
	{
		using enum Components::Camera::Direction;
		auto& input = Input::Instance();

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
		if (input.IsJustPressed(Key::Space))
		{
			camera.moveSpeed *= 10;
		}
		if (input.IsJustReleased(Key::Space))
		{
			camera.moveSpeed /= 10.f;
		}
	}
	void FpsCameraController::UpdateByMouseInput(Components::Camera& camera)
	{
		auto& input = Input::Instance();

		if (input.DidScroll())
		{
			camera.Zoom(input.ScrollOffs());
		}

		if (input.IsDown(Button::Left))
		{
			camera.Rotate(input.CursorYOffs(), -input.CursorXOffs());
		}
	}

	void EditorCameraController::UpdateByKeyInput(Components::Camera& camera, float delta)
	{
		auto& input = Input::Instance();
		/*using enum Components::Camera::Direction;

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
		}*/
		if (input.IsJustPressed(Key::Space))
		{
			camera.moveSpeed *= 10;
		}
		if (input.IsJustReleased(Key::Space))
		{
			camera.moveSpeed /= 10.f;
		}
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
			auto a = dir.x / glm::cos(glm::radians(camera.pitch));
			auto yaw = glm::degrees(glm::acos(a));
			camera.SetRotation(pitch, yaw);
			camera.SetPosition(center + -camera.front * distance);
		}
		if (input.DidScroll())
		{
			camera.Move(Components::Camera::Direction::Forward, input.ScrollOffs() * 50);
		}

		if (input.IsDown(Button::Left))
		{
			camera.Rotate(-input.CursorYOffs(), input.CursorXOffs());
			camera.SetPosition(center + -camera.front * distance);
		}
	}
}

