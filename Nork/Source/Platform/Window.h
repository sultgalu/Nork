#pragma once

#include "Core/Input.h"

namespace Nork
{
	class StateListener;

	template<typename T>
	class _Window
	{
	public:
		_Window(int width, int height);
		~_Window();
		
		void Refresh();
		void Close();
		void SetSize(int width, int height);

		bool IsRunning();
		inline Input::State& GetInputState() { return inputState; }
		inline Event::Receiver& GetInputEvents() { return reg.GetReceiver(); }
		T& GetData();
	private:
		void SetupEventCallbacks();
		Event::Dispatcher reg;
		Input::StateListener inputState = Input::StateListener(reg.GetReceiver()); // state will update automatically
	};

#ifdef _WIN64
	typedef _Window<GLFWwindow> Window;
#else
	typedef _Window<int> Window;
#endif
}