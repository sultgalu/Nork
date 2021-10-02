#include "pch.h"
#include "CameraController.h"

namespace Nork
{
	void CameraController::HandleKeyUp(const Event& event)
	{	
		auto& ev = event.As<Events::KeyUp>();
	}
	void CameraController::HandleKeyDown(const Event& event)
	{
		auto& ev = event.As<Events::KeyDown>();
	}
	void CameraController::HandleScroll(const Event& event)
	{
		auto& ev = event.As<Events::MouseScroll>();
	}
	void CameraController::HandleMouseMove(const Event& event)
	{
		auto& ev = event.As<Events::MouseMove>();
	}
	void CameraController::HandleMouseDown(const Event& event)
	{
		auto& ev = event.As<Events::MouseDown>();
	}
	void CameraController::HandleMouseUp(const Event& event)
	{
		auto& ev = event.As<Events::MouseUp>();
	}
	void CameraController::SubscribeEvents(EventManager& em)
	{
		em.Subscribe<Events::KeyUp>(&CameraController::HandleKeyUp, this);
		em.Subscribe<Events::KeyDown>(&CameraController::HandleKeyDown, this);
		em.Subscribe<Events::MouseScroll>(&CameraController::HandleScroll, this);
		em.Subscribe<Events::MouseMove>(&CameraController::HandleMouseMove, this);
		em.Subscribe<Events::MouseDown>(&CameraController::HandleMouseDown, this);
		em.Subscribe<Events::MouseUp>(&CameraController::HandleMouseUp, this);
	}
}

