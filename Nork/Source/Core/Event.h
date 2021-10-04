#pragma once

#include "Platform/Input.h"

namespace Nork
{
	// For switch statements to work, and for compile-time evaluation of GetType(), use hard coded values in each derived Events instead of typeid() (which evaluates at runtime)
	// Also now every GetType() costs a v-table lookup instead of being compile-time evaluated (can be optimized later)

	struct Event
	{
		inline virtual const size_t GetType() const { return typeid(*this).hash_code(); }

		template<std::derived_from<Event> T>
		inline static constexpr size_t TypeOf() { return typeid(T).hash_code(); }

		template<std::derived_from<Event> T>
		inline const bool IsType() const { return typeid(T).hash_code() == GetType(); }

		template<std::derived_from<Event> T>
		inline const constexpr T& As() const { return static_cast<const T&>(*this); }
	};

	// These events get dispatched when events get polled.
	struct InputEvent : public Event {};

	// Contrary to basic Event types, these get dispatched immediately, bypassing the event queue.
	struct AppEvent : public Event {};

	namespace Events
	{
		using namespace Input;
		struct KeyEvent : InputEvent
		{
			KeyEvent(Key key) : key(key) {}
			const Key key;
		};

		struct KeyDown : public KeyEvent
		{
			KeyDown(Key key) : KeyEvent(key) {}
		};

		struct KeyUp : public KeyEvent
		{
			KeyUp(Key key) : KeyEvent(key) {}
		};

		struct Type : public InputEvent
		{
			Type(unsigned int c) : character(c) {}
			const unsigned int character;
		};

		struct CursorLeftWindow : public InputEvent {};
		struct CursorEnteredWindow : public InputEvent {};

		struct MonitorConnect : public InputEvent {};
		struct MonitorDisconnect : public InputEvent {};

		struct MouseEvent : InputEvent {};
		struct MouseButtonEvent : MouseEvent
		{
			MouseButtonEvent(MouseButton button) : button(button) {}
			MouseButton button;
		};

		struct MouseDown : public MouseButtonEvent
		{
			MouseDown(MouseButton button) : MouseButtonEvent(button) {}
		};

		struct MouseUp : public MouseButtonEvent
		{
			MouseUp(MouseButton button) : MouseButtonEvent(button) {}
		};

		struct MouseMove : public MouseEvent
		{
			MouseMove(double offsetX, double offsetY) : offsetX(offsetX), offsetY(offsetY) {}
			double offsetX, offsetY;
		};

		struct MouseScroll : public InputEvent
		{
			MouseScroll(double offset) : offset(offset) {}
			double offset;
		};

		struct WindowInFocus : public InputEvent {};
		struct WindowOutOfFocus : public InputEvent {};

		struct WindowResize : public InputEvent
		{
			WindowResize(int width, int height) : width(width), height(height) {}
			int width, height;
		};

		struct WindowClose : public InputEvent
		{
			WindowClose(bool calledByWindow) : calledByWindow(calledByWindow) {}
			bool calledByWindow;
		};

		struct OnUpdate : public AppEvent {};
		struct Updated : public AppEvent {};
		struct OnRenderUpdate : public AppEvent {};
		struct RenderUpdated : public AppEvent {};
		struct OnPhysicsUpdate : public AppEvent {};
		struct PhysicsUpdated : public AppEvent {};
	}

	class EventQueue
	{
	public:
		template<std::derived_from<Event> T>
		void Enqueue(const T ev)
		{
			this->events.push(new T(ev));
		}
		std::unique_ptr<Event> Dequeue()
		{
			auto e = this->events.front();
			this->events.pop();
			return std::unique_ptr<Event>(e);
		}
		size_t GetCount()
		{
			return this->events.size();
		}
	private:
		std::queue<Event*> events;
	};

	class EventDispatcher
	{
	public:
		template<std::derived_from<Event> T>
		void Subscribe(std::function<void(const Event&)> callback)
		{
			this->subscribers[Event::TypeOf<T>()].push_back(callback);
		}

		template<std::derived_from<Event> T>
		void Dispatch(const T& e)
		{
			for (auto& cb : this->subscribers[e.GetType()])
			{
				cb(e);
			}
		}

	private:
		std::unordered_map<size_t, std::vector<std::function<void(const Event&)>>> subscribers;
	};

	class EventManager
	{
	public:
		template<std::derived_from<InputEvent> T>
		void RaiseEvent(T e)
		{
			this->queue.Enqueue(e);
		}

		template<std::derived_from<AppEvent> T>
		void RaiseEvent(T e)
		{
			this->dispatcher.Dispatch(e);
		}

		void PollEvents()
		{
			size_t count = queue.GetCount();
			for (size_t i = 0; i < count; i++)
			{
				this->dispatcher.Dispatch(*queue.Dequeue().get());
			}
		}

		template<std::derived_from<Event> T>
		void Subscribe(std::function<void(const Event&)> callback)
		{
			this->dispatcher.Subscribe<T>(callback);
		}

		template<std::derived_from<Event> T, typename F, typename M>
		void Subscribe(F func, M member)
		{
			this->dispatcher.Subscribe<T>(std::bind(func, member, std::placeholders::_1));
		}

	private:
		EventQueue queue;
		EventDispatcher dispatcher;
	};
}

