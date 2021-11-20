#pragma once
#include "Core/Event.h"
#include "Utils/Input.h"

namespace Nork
{
	enum class KeyState : uint8_t
	{
		None = 0,

		Up = 1 << 0,
		Down = 1 << 1,
		Repeat = 1 << 2,
		Pressed = 1 << 3,
		Released = 1 << 4,
		Repeated = 1 << 5,
	};
	enum class MouseButtonState : uint8_t
	{
		None = 0,

		Up = 1 << 0,
		Down = 1 << 1,
		Repeat = 1 << 2,
		Pressed = 1 << 3,
		Released = 1 << 4,
		Repeated = 1 << 5,
	};
	enum class CursorState : uint16_t
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

	class InputState: Template::Types::OnlyConstruct
	{
	public:
		InputState(Event::Receiver& receiver)
		{
			SetupByGLFW();

			MetaLogger().Debug("Should write this completely");

			receiver.Subscribe(std::function([&](const Event::Types::KeyDown& e)
				{
					keys[e.AsInt()] |= KeyState::Down;
					keys[e.AsInt()] &= ~KeyState::Up;
				}));
			receiver.Subscribe(std::function([&](const Event::Types::KeyUp& e)
				{
					keys[e.AsInt()] |= KeyState::Up;
					keys[e.AsInt()] &= ~KeyState::Down;
				}));
			receiver.Subscribe(std::function([&](const Event::Types::MouseDown& e)
				{
					mouseButtons[e.AsInt()] |= MouseButtonState::Down;
					mouseButtons[e.AsInt()] &= ~MouseButtonState::Up;
				}));
			receiver.Subscribe(std::function([&](const Event::Types::MouseUp& e)
				{
					mouseButtons[e.AsInt()] |= MouseButtonState::Up;
					mouseButtons[e.AsInt()] &= ~MouseButtonState::Down;
				}));
		}

		inline KeyState Get(Key key)
		{
			return keys[ToInt(key)];
		}
		template<KeyState State>
		inline bool Is(Key key)
		{
			return (keys[ToInt(key)] & State) == State;
		}
		inline bool Is(Key key, KeyState state)
		{
			return (keys[ToInt(key)] & state) == state;
		}
		inline bool IsAny(Key key, KeyState state)
		{
			return (keys[ToInt(key)] & state) != KeyState::None;
		}
		inline bool IsNone(Key key, KeyState state)
		{
			return (keys[ToInt(key)] & state) == KeyState::None;
		}

		inline MouseButtonState Get(MouseButton button)
		{
			return mouseButtons[ToInt(button)];
		}
		template<MouseButtonState State>
		inline bool Is(MouseButton button)
		{
			return (mouseButtons[ToInt(button)] & State) == State;
		}
		inline bool Is(MouseButton button, MouseButtonState state)
		{
			return (mouseButtons[ToInt(button)] & state) == state;
		}
		inline bool IsAny(MouseButton button, MouseButtonState state)
		{
			return (mouseButtons[ToInt(button)] & state) != MouseButtonState::None;
		}
		inline bool IsNone(MouseButton button, MouseButtonState state)
		{
			return (mouseButtons[ToInt(button)] & state) == MouseButtonState::None;
		}
	private:
		void SetupByGLFW()
		{

		}
	private:
		KeyState keys[ToInt(Key::Max)];
		MouseButtonState mouseButtons[ToInt(MouseButton::Max)];
	};
}