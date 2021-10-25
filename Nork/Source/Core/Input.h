#pragma once
#include "Core/Event.h"
#include "Platform/Input.h"

namespace Nork::Input
{
	// Wraps an EventManager that should only dispatch input events
	// TEMPLATE EventManager??? EventManager<InputEvent>
	// It adds convinience methods such as IsKeyDown
	/*class Input
	{
	public:
		Input(Event::Receiver& em) : eventMan(em)
		{
			Subscribe();
		}
		inline bool IsKeyDown(Types::Key key) { return keys[ToInt(key)]; }
		inline bool IsMouseButtonDown(Types::MouseButton button) { return buttons[ToInt(button)]; }
		inline Event::Receiver& GetEventDispatcher() { return eventMan; }
	private:
		inline void Subscribe()
		{
			eventMan.Subscribe<Event::Types::KeyUp>(&Input::HandleKeyEvent, this);
			eventMan.Subscribe<Event::Types::KeyDown>(&Input::HandleKeyEvent, this);

			eventMan.Subscribe<Event::Types::MouseDown>(&Input::HandleMouseButtonEvent, this);
			eventMan.Subscribe<Event::Types::MouseUp>(&Input::HandleMouseButtonEvent, this);
		}
		inline void HandleKeyEvent(const Event::Types::Base& ev)
		{
			keys[Types::ToInt(ev.As<Event::Types::KeyEvent>().key)].flip();
		}
		inline void HandleMouseButtonEvent(const Event::Types::Base& ev)
		{
			buttons[Types::ToInt(ev.As<Event::Types::MouseButtonEvent>().button)].flip();
		}
		std::bitset<Types::ToInt(Types::Key::Max)> keys;
		std::bitset<Types::ToInt(Types::MouseButton::Max)> buttons;
		Event::Receiver& eventMan;
	};*/

	class State
	{
	public:
		enum class Key : uint8_t
		{
			None = 0,

			Up = 1 << 0,
			Down = 1 << 1,
			Repeat = 1 << 2,
			Pressed = 1 << 3,
			Released = 1 << 4,
			Repeated = 1 << 5,
		};
		enum class MouseButton : uint8_t
		{
			None = 0,

			Up = 1 << 0,
			Down = 1 << 1,
			Repeat = 1 << 2,
			Pressed = 1 << 3,
			Released = 1 << 4,
			Repeated = 1 << 5,
		};
		enum class Mouse : uint16_t
		{
			None = 0,

			Still = 1 << 0,
			Moved = 1 << 1,
			Moving = 1 << 2,
			Stopped = 1 << 3,

			MovingUp = 1 << 4,
			MovingDown = 1 << 5,
			MovingLeft = 1 << 6,
			MovingRight = 1 << 7,

			MovedUp = 1 << 8,
			MovedDown = 1 << 9,
			MovedLeft = 1 << 10,
			MovedRight = 1 << 11,

			StoppedUp = 1 << 12,
			StoppedDown = 1 << 13,
			StoppedLeft = 1 << 14,
			StoppedRight = 1 << 15,
		};
		enum class Character
		{
			None = 0,

			Off = 1 << 0,
			Typed = 1 << 1,

			Repeated = 1 << 2,
			StoppedRepeate = 1 << 3,

			HeldNotRepeated = 1 << 4,
			ReleasedBeforeRepeate = 1 << 5,
		};

		inline Key Get(Types::Key key)
		{
			return keys[Types::ToInt(key)];
		}
		template<State::Key State>
		inline bool Is(Types::Key key)
		{
			return (keys[ToInt(key)] & State) == State;
		}
		inline bool Is(Types::Key key, Key state)
		{
			return (keys[ToInt(key)] & state) == state;
		}
		inline bool IsAny(Types::Key key, Key state)
		{
			return (keys[ToInt(key)] & state) != Key::None;
		}
		inline bool IsNone(Types::Key key, Key state)
		{
			return (keys[ToInt(key)] & state) == Key::None;
		}

		inline MouseButton Get(Types::MouseButton button)
		{
			return mouseButtons[Types::ToInt(button)];
		}
		template<State::MouseButton State>
		inline bool Is(Types::MouseButton button)
		{
			return (mouseButtons[ToInt(button)] & State) == State;
		}
		inline bool Is(Types::MouseButton button, MouseButton state)
		{
			return (mouseButtons[ToInt(button)] & state) == state;
		}
		inline bool IsAny(Types::MouseButton button, MouseButton state)
		{
			return (mouseButtons[ToInt(button)] & state) != MouseButton::None;
		}
		inline bool IsNone(Types::MouseButton button, MouseButton state)
		{
			return (mouseButtons[ToInt(button)] & state) == MouseButton::None;
		}

		/*inline Mouse GetMouse()
		{
			return mouse;
		}*/

		~State()
		{
			Logger::Debug("STATE DYING.");
		}

	protected:
		Key keys[Types::ToInt(Types::Key::Max)];
		MouseButton mouseButtons[Types::ToInt(Types::MouseButton::Max)];
		//Mouse mouse;
	};

	using KeyState = State::Key;
	using MouseButtonState = State::MouseButton;
	using MouseState = State::Mouse;

	class StateListener : public State
	{
	public:
		StateListener(Event::Receiver& receiver) : State()
		{
			SetupCallbacks(receiver);
			QueryCurrentState();
		}
	private:
		void QueryCurrentState();
		void SetupCallbacks(Event::Receiver& receiver)
		{
			MetaLogger().Debug("Should write this completely");

			receiver.Subscribe(std::function([&](const Event::Types::KeyDown& e)
				{
					this->keys[e.AsInt()] |= Key::Down;
					this->keys[e.AsInt()] &= ~Key::Up;
				}));
			receiver.Subscribe(std::function([&](const Event::Types::KeyUp& e)
				{
					this->keys[e.AsInt()] |= Key::Up;
					this->keys[e.AsInt()] &= ~Key::Down;
				}));
			receiver.Subscribe(std::function([&](const Event::Types::MouseDown& e)
				{
					this->mouseButtons[e.AsInt()] |= MouseButton::Down;
					this->mouseButtons[e.AsInt()] &= ~MouseButton::Up;
				}));
			receiver.Subscribe(std::function([&](const Event::Types::MouseUp& e)
				{
					this->mouseButtons[e.AsInt()] |= MouseButton::Up;
					this->mouseButtons[e.AsInt()] &= ~MouseButton::Down;
				}));
		}
	};
}