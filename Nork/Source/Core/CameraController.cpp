#include "pch.h"
#include "CameraController.h"

namespace Nork
{
	static bool isVerticalMovement = true;

	void CameraController::OnUpdate(float delta)
	{
		using namespace Input;
		using enum Components::Camera::Direction;
		if (input.IsKeyDown(Key::W))
		{
			auto dir = isVerticalMovement ? Up : Forward;
			camera->Move(dir, delta);
		}
		if (input.IsKeyDown(Key::S))
		{
			auto dir = isVerticalMovement ? Down : Backward;
			camera->Move(dir, delta);
		}
		if (input.IsKeyDown(Key::A))
		{
			camera->Move(Left, delta);
		}
		if (input.IsKeyDown(Key::D))
		{
			camera->Move(Rigth, delta);
		}
	}
	void CameraController::SetupInputHandling()
	{
		input.GetEventManager().Subscribe<Events::KeyUp>(&CameraController::HandleKeyUp, this);
		input.GetEventManager().Subscribe<Events::KeyDown>(&CameraController::HandleKeyDown, this);
		input.GetEventManager().Subscribe<Events::MouseScroll>(&CameraController::HandleScroll, this);
		input.GetEventManager().Subscribe<Events::MouseMove>(&CameraController::HandleMouseMove, this);
		input.GetEventManager().Subscribe<Events::MouseDown>(&CameraController::HandleMouseDown, this);
		input.GetEventManager().Subscribe<Events::MouseUp>(&CameraController::HandleMouseUp, this);
		input.GetEventManager().Subscribe<Events::OnUpdate>(&CameraController::HandleOnUpdate, this);
	}

	void CameraController::HandleKeyUp(const Event& event)
	{
		using enum Input::Key;

		switch (event.As<Events::KeyDown>().key)
		{
		case Shift:
			isVerticalMovement = true;
			break;
		case Space:
			camera->moveSpeed /= 10.0f;
			break;
		default:
			break;
		}
	}
	void CameraController::HandleKeyDown(const Event& event)
	{
		using enum Input::Key;

		switch (event.As<Events::KeyDown>().key)
		{
		case Shift:
			isVerticalMovement = false;
			break;
		case Space:
			camera->moveSpeed *= 10.0f;
			break;
		default:
			break;
		}
	}
	void CameraController::HandleScroll(const Event& event)
	{
		auto& ev = event.As<Events::MouseScroll>();

		camera->Zoom(ev.offset);
	}
	void CameraController::HandleMouseMove(const Event& event)
	{
		static double x = 0;
		static double y = 0;

		auto& ev = event.As<Events::MouseMove>();

		double offsX = x - ev.offsetX;
		double offsY = y - ev.offsetY;

		if (input.IsMouseButtonDown(Input::MouseButton::Left))
		{
			camera->Rotate(-offsY, offsX);
		}
		x = ev.offsetX;
		y = ev.offsetY;
	}
	void CameraController::HandleMouseDown(const Event& event)
	{
		auto& ev = event.As<Events::MouseDown>();
	}
	void CameraController::HandleMouseUp(const Event& event)
	{
		auto& ev = event.As<Events::MouseUp>();
	}
	void CameraController::HandleOnUpdate(const Event& event)
	{
		// auto& ev = event.As<Events::OnUpdate>();
	}
}

