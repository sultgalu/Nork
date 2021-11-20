#include "pch.h"
#include "CameraController.h"
#include "App/Application.h"

namespace Nork
{
	void CameraController::Update(float delta)
	{
		using enum Components::Camera::Direction;

		auto& input = Application::Get().inputState;

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
	void CameraController::SetupInputHandling(Event::Receiver& receiver)
	{
		receiver.Subscribe<Event::Types::MouseScroll>(&CameraController::HandleScroll, this);
		receiver.Subscribe<Event::Types::MouseMove>(&CameraController::HandleMouseMove, this);
	}
	void CameraController::HandleScroll(const Event::Types::MouseScroll& event)
	{
		camera.Zoom(event.offset);
	}
	void CameraController::HandleMouseMove(const Event::Types::MouseMove& event)
	{
		static double x = 0;
		static double y = 0;

		double offsX = x - event.offsetX;
		double offsY = y - event.offsetY;

		auto& input = Application::Get().inputState;
		if (input.Is(MouseButton::Left, MouseButtonState::Down))
		{
			camera.Rotate(-offsY, offsX);
		}
		x = event.offsetX;
		y = event.offsetY;
	}
}

