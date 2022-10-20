#pragma once
import Nork.Utils;

namespace Nork
{
	class Input: Template::Types::OnlyConstruct
	{
	public:
		Input(GLFWwindow* winPtr);
		void Update(); // call before input polling

		bool IsDown(Key key) const { return keys.test(static_cast<int>(key)); }
		bool IsDown(Button button) const { return buttons.test(static_cast<int>(button)); }
		bool IsUp(Key key) const { return !keys.test(static_cast<int>(key)); }
		bool IsUp(Button button) const { return !buttons.test(static_cast<int>(button)); }

		bool IsJustPressed(Key key) const { return keyChanged.test(static_cast<int>(key)) && keys.test(static_cast<int>(key)); }
		bool IsJustPressed(Button key) const { return buttonChanged.test(static_cast<int>(key)) && buttons.test(static_cast<int>(key)); }
		bool IsJustReleased(Key key) const { return keyChanged.test(static_cast<int>(key)) && !keys.test(static_cast<int>(key)); }
		bool IsJustReleased(Button key) const { return buttonChanged.test(static_cast<int>(key)) && !buttons.test(static_cast<int>(key)); }

		bool IsRepeated(Key key) const { return keysRepeated.test(static_cast<int>(key)); }

		double CursorX() const { return cursorX; }
		double CursorY() const { return cursorY; }
		double CursorXOffs() const { return cursorX - prevCursorX; }
		double CursorYOffs() const { return cursorY - prevCursorY; }
		bool DidCursorMove() const { return cursorX != prevCursorX || cursorY != prevCursorY; }

		double ScrollOffs() const { return scrollOffset; }
		bool DidScroll() const { return scrollOffset != 0; }

		bool DidCursorJustEnter() const { return cursorEntered; }
		bool DidCursorJustLeave() const { return cursorLeft; }

		bool ShouldWindowClose() const { return windowShouldClose; }

		const std::vector<unsigned int>& TypedCharacters() const { return typedChars; }
		const std::bitset<ToInt(Key::Max)>& KeysDown() const { return keys; }
 	private:
		std::bitset<ToInt(Key::Max)> keys;
		std::bitset<ToInt(Key::Max)> keyChanged;
		std::bitset<ToInt(Key::Max)> keysRepeated;
		std::bitset<ToInt(Button::Max)> buttons;
		std::bitset<ToInt(Button::Max)> buttonChanged;
		std::vector<unsigned int> typedChars;

		double scrollOffset = 0;
		double cursorX = 0, cursorY = 0;
		double prevCursorX = 0, prevCursorY = 0;

		bool windowCloseRequested = false;

		bool cursorEntered = false;
		bool cursorLeft = false;

		bool windowShouldClose = false;
	};
}