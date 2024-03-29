#pragma once

namespace Nork
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
		Z = GLFW_KEY_Z,
		U = GLFW_KEY_U,
		I = GLFW_KEY_I,
		O = GLFW_KEY_O,

		F = GLFW_KEY_F,
		G = GLFW_KEY_G,
		H = GLFW_KEY_H,
		J = GLFW_KEY_J,
		K = GLFW_KEY_K,
		L = GLFW_KEY_L,

		N = GLFW_KEY_N,
		M = GLFW_KEY_M,

		F1 = GLFW_KEY_F1,
		F2 = GLFW_KEY_F2,
		F3 = GLFW_KEY_F3,
		F4 = GLFW_KEY_F4,
		F5 = GLFW_KEY_F5,
		F6 = GLFW_KEY_F6,
		F7 = GLFW_KEY_F7,
		F8 = GLFW_KEY_F8,
		F9 = GLFW_KEY_F9,
		F10 = GLFW_KEY_F10,

		Max = GLFW_KEY_LAST
	};

	enum class Button
	{
		Left = GLFW_MOUSE_BUTTON_LEFT,
		Right = GLFW_MOUSE_BUTTON_RIGHT,
		Middle = GLFW_MOUSE_BUTTON_MIDDLE,
		
		Max = 3
	};

	inline std::string ToString(Key key) { return std::to_string(std::to_underlying(key)); }
	inline std::string ToString(Button button) { return std::to_string(std::to_underlying(button)); }
	inline constexpr int ToInt(Key key) { return std::to_underlying(key); }
	inline constexpr int ToInt(Button button) { return std::to_underlying(button); }

	static_assert(ToInt(Key::Max) < 1024);
	static_assert(ToInt(Button::Max) < 16);
}