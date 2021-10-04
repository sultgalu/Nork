#pragma once
#include "Core/Event.h"
#include "Platform/Input.h"

namespace Nork::Input
{
	// Wraps an EventManager that should only dispatch input events
	// TEMPLATE EventManager??? EventManager<InputEvent>
	// It adds convinience methods such as IsKeyDown
	class Input
	{
	public:
		Input() : eventMan(EventManager())
		{
			eventMan.Subscribe<Events::KeyUp>(&Input::HandleKeyEvent, this);
			eventMan.Subscribe<Events::KeyDown>(&Input::HandleKeyEvent, this);

			eventMan.Subscribe<Events::MouseDown>(&Input::HandleMouseButtonEvent, this);
			eventMan.Subscribe<Events::MouseUp>(&Input::HandleMouseButtonEvent, this);
		}
		inline bool IsKeyDown(Key key) { return keys[ToInt(key)]; }
		inline bool IsMouseButtonDown(MouseButton button) { return buttons[ToInt(button)]; }
		inline EventManager& GetEventManager() { return eventMan; }
	private:
		inline void HandleKeyEvent(const Event& ev)
		{
			keys[ToInt(ev.As<Events::KeyEvent>().key)].flip();
		}
		inline void HandleMouseButtonEvent(const Event& ev)
		{
			buttons[ToInt(ev.As<Events::MouseButtonEvent>().button)].flip();
		}
		std::bitset<ToInt(Key::Max)> keys;
		std::bitset<ToInt(MouseButton::Max)> buttons;
		EventManager eventMan;
	};
}