#pragma once

namespace Nork::Input::Types
{
	enum class Key
	{
		Up = GLFW_KEY_UP,
		Down = GLFW_KEY_DOWN,
		Left = GLFW_KEY_LEFT,
		Right = GLFW_KEY_RIGHT,

		Tab = GLFW_KEY_TAB,
		Space = GLFW_KEY_SPACE,
		Enter = GLFW_KEY_ENTER,
		Esc = GLFW_KEY_ESCAPE,
		Super = GLFW_KEY_LEFT_SUPER,

		Shift = GLFW_KEY_LEFT_SHIFT,
		RightShift = GLFW_KEY_RIGHT_SHIFT,
		Ctrl = GLFW_KEY_LEFT_CONTROL,
		CtrlRight = GLFW_KEY_RIGHT_CONTROL,
		Alt = GLFW_KEY_LEFT_ALT,
		RightAlt = GLFW_KEY_RIGHT_ALT,

		W = GLFW_KEY_W,
		A = GLFW_KEY_A,
		S = GLFW_KEY_S,
		D = GLFW_KEY_D,
		Q = GLFW_KEY_Q,
		E = GLFW_KEY_E,
		R = GLFW_KEY_R,
		T = GLFW_KEY_T,
		M = GLFW_KEY_M,

		Max = GLFW_KEY_LAST
	};

	enum class MouseButton
	{
		Left = GLFW_MOUSE_BUTTON_LEFT,
		Right = GLFW_MOUSE_BUTTON_RIGHT,
		Middle = GLFW_MOUSE_BUTTON_MIDDLE,
		
		Max = 3
	};

	inline std::string ToString(Key key) { return std::to_string(std::to_underlying(key)); }
	inline std::string ToString(MouseButton button) { return std::to_string(std::to_underlying(button)); }
	inline constexpr int ToInt(Key key) { return std::to_underlying(key); }
	inline constexpr int ToInt(MouseButton button) { return std::to_underlying(button); }

	static_assert(ToInt(Key::Max) < 1024);
	static_assert(ToInt(MouseButton::Max) < 16);
}

namespace Nork::Input
{
	using KeyType = Types::Key;
	using MouseButtonType = Types::MouseButton;
}