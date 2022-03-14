#include "pch.h"
#include "CameraController.h"
#include "App/Application.h"

namespace Nork
{
	void CameraController::Update(float delta)
	{
		using enum Components::Camera::Direction;

		auto& input = Application::Get().inputState;
		auto& camera = scene.GetMainCamera();

		if (input.Is(Key::W, KeyState::Down))
		{
			auto dir = input.Is(Key::Shift, KeyState::Down) ? Up : Forward;
			camera.Move(dir, delta);
		}
		if (input.Is(Key::S, KeyState::Down))
		{
			auto dir = input.Is(Key::Shift, KeyState::Down) ? Down : Backward;
			camera.Move(dir, delta);
		}
		if (input.Is(Key::A, KeyState::Down))
		{
			camera.Move(Left, delta);
		}
		if (input.Is(Key::D, KeyState::Down))
		{
			camera.Move(Rigth, delta);
		}

		static float baseSpeed = camera.moveSpeed;
		camera.moveSpeed = input.Is(Key::Space, KeyState::Down) ? baseSpeed * 10.0f : baseSpeed;
	}
	void CameraController::SetupInputHandling(Receiver& receiver)
	{
		receiver.Subscribe<MouseScrollEvent>(&CameraController::HandleScroll, this);
		receiver.Subscribe<MouseMoveEvent>(&CameraController::HandleMouseMove, this);
	}
	void CameraController::HandleScroll(const MouseScrollEvent& event)
	{
		scene.GetMainCamera().Zoom(event.offset);
	}
	void CameraController::HandleMouseMove(const MouseMoveEvent& event)
	{
		static double x = 0;
		static double y = 0;

		double offsX = x - event.offsetX;
		double offsY = y - event.offsetY;

		auto& input = Application::Get().inputState;
		if (input.Is(MouseButton::Left, MouseButtonState::Down))
		{
			scene.GetMainCamera().Rotate(-offsY, offsX);
		}
		x = event.offsetX;
		y = event.offsetY;
	}
}

