#pragma once

#include "Input.h"
#include "Components/Camera.h"

namespace Nork
{
	class BuiltInScript
	{
	public:
		virtual void OnUpdate(float delta) {}
	};

	class CameraController : public BuiltInScript
	{
	public:
		CameraController(Input::Input& input, std::shared_ptr<Components::Camera> cam = std::shared_ptr<Components::Camera>())
			: camera(cam), input(input)
		{
			SetupInputHandling();
		}
		void OnUpdate(float delta) override;
	private:
		void SetupInputHandling();
		void HandleKeyUp(const Event& event);
		void HandleKeyDown(const Event& event);
		void HandleScroll(const Event& event);
		void HandleMouseMove(const Event& event);
		void HandleMouseDown(const Event& event);
		void HandleMouseUp(const Event& event);
		void HandleOnUpdate(const Event& event);
	public:
		std::shared_ptr<Components::Camera> camera;
	private:
		Input::Input& input;
	};
}