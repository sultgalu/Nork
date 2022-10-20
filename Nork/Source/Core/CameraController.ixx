export module Nork.Core:CameraController;

export import :InputState;
import Nork.Components;
import Nork.Scene;

export namespace Nork
{
	class CameraController
	{
	public:
		CameraController()
		{}
		static void UpdateByKeyInput(Components::Camera&, float delta);
		static void UpdateByMouseInput(Components::Camera&, float delta);
	};
}