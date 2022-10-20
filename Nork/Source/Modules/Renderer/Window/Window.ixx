export module Nork.Renderer:Window;

export namespace Nork::Renderer
{
	struct WindowSetup
	{
		int width, height;
		std::string label;
	};
	struct Context
	{
		void ParseSetup(WindowSetup setup)
		{
			width = setup.width;
			height = setup.height;
			label = setup.label;
		}
		int refreshRate;
		int width, height;
		std::string label;
		GLFWwindow* glfwWinPtr;
	};

	class Window
	{
	public:
		Window(WindowSetup setup);
		void Close();
		void Resize(int w, int h);
		void Refresh();
		inline void PollEvents() { glfwPollEvents(); }
		inline const Context& GetContext() { return context; }
	public: // should be called from outside
		void OnResize(int newX, int newY);
	private:
		Context context;
	};
}


/*virtual void OnClose();
		virtual void OnResize(int width, int height);
		virtual void OnKeyDown(Key key);
		virtual void OnKeyUp(Key key);
		virtual void OnMouseDown(MouseButton button);
		virtual void OnMouseUp(MouseButton button);
		virtual void OnCursorMove(double x, double y);
		virtual void OnCursorEnter();
		virtual void OnCursorLeave();
		virtual void OnScroll(double offset);
		virtual void OnChar(unsigned int c);
		virtual void OnMonitorConnect();
		virtual void OnMonitorDisconnect();
		virtual void OnWindowEnterFocus();
		virtual void OnWindowLeaveFocus();*/