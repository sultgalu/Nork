#pragma once

#include "Event.h"
#include "Components/Camera.h"

namespace Nork
{
	class CameraController
	{
	public:
		CameraController(EventManager& em, std::shared_ptr<Components::Camera> cam = std::shared_ptr<Components::Camera>())
			: camera(cam)
		{

		}
	private:
		void SubscribeEvents(EventManager& em);
		void HandleKeyUp(const Event& event);
		void HandleKeyDown(const Event& event);
		void HandleScroll(const Event& event);
		void HandleMouseMove(const Event& event);
		void HandleMouseDown(const Event& event);
		void HandleMouseUp(const Event& event);
	public:
		std::shared_ptr<Components::Camera> camera;
	};
}