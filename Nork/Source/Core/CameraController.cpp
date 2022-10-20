module Nork.Core;

import Application;

namespace Nork
{
	void CameraController::UpdateByKeyInput(Components::Camera& camera, float delta)
	{
		using enum Components::Camera::Direction;
		auto& input = Application::Get().engine.window.Input();

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
	void CameraController::UpdateByMouseInput(Components::Camera& camera, float delta)
	{
		auto& input = Application::Get().engine.window.Input();

		if (input.DidScroll())
		{
			camera.Zoom(input.ScrollOffs());
		}

		if (input.IsDown(Button::Left))
		{
			camera.Rotate(input.CursorYOffs(), -input.CursorXOffs());
		}
	}
}

