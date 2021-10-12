#include "pch.h"
#include "CameraController.h"

namespace Nork
{
	void CameraController::Update(float delta)
	{
		using namespace Input;

		using enum Components::Camera::Direction;

		if (input.Is(KeyType::W, KeyState::Down))
		{
			auto dir = input.Is(KeyType::Shift, KeyState::Down) ? Up : Forward;
			camera.Move(dir, delta);
		}
		if (input.Is(KeyType::S, KeyState::Down))
		{
			auto dir = input.Is(KeyType::Shift, KeyState::Down) ? Down : Backward;
			camera.Move(dir, delta);
		}
		if (input.Is(KeyType::A, KeyState::Down))
		{
			camera.Move(Left, delta);
		}
		if (input.Is(KeyType::D, KeyState::Down))
		{
			camera.Move(Rigth, delta);
		}

		static float baseSpeed = camera.moveSpeed;
		camera.moveSpeed = input.Is(KeyType::Space, KeyState::Down) ? baseSpeed * 10.0f : baseSpeed;
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
		using namespace Input;
		
		static double x = 0;
		static double y = 0;

		double offsX = x - event.offsetX;
		double offsY = y - event.offsetY;

		if (input.Is(MouseButtonType::Left, MouseButtonState::Down))
		{
			camera.Rotate(-offsY, offsX);
		}
		x = event.offsetX;
		y = event.offsetY;
	}
}

