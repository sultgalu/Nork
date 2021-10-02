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
	struct QueuedEvent : public Event {};

	// Contrary to basic Event types, these get dispatched immediately, bypassing the event queue.
	struct ImmediateEvent : public Event {};

	namespace Events
	{
		using namespace Input;
		struct KeyDown : public QueuedEvent
		{
			KeyDown(Key key) : key(key) {}
			const Key key;
		};

		struct KeyUp : public QueuedEvent
		{
			KeyUp(Key key) : key(key) {}
			const Key key;
		};

		struct MouseDown : public QueuedEvent
		{
			MouseDown(MouseButton button) : button(button) {}
			MouseButton button;
		};

		struct MouseUp : public QueuedEvent
		{
			MouseUp(MouseButton button) : button(button) {}
			MouseButton button;
		};

		struct MouseMove : public QueuedEvent
		{
			MouseMove(double offsetX, double offsetY) : offsetX(offsetX), offsetY(offsetY) {}
			double offsetX, offsetY;
		};

		struct MouseScroll : public QueuedEvent
		{
			MouseScroll(double offset) : offset(offset) {}
			double offset;
		};

		struct WindowResize : public QueuedEvent
		{
			WindowResize(int width, int height) : width(width), height(height) {}
			int width, height;
		};

		struct WindowClose : public QueuedEvent
		{
			WindowClose(bool calledByWindow) : calledByWindow(calledByWindow) {}
			bool calledByWindow;
		};

		struct OnUpdate : public ImmediateEvent {};
		struct Updated : public ImmediateEvent {};
		struct OnRenderUpdate : public ImmediateEvent {};
		struct RenderUpdated : public ImmediateEvent {};
		struct OnPhysicsUpdate : public ImmediateEvent {};
		struct PhysicsUpdated : public ImmediateEvent {};
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
		template<std::derived_from<QueuedEvent> T>
		void RaiseEvent(T e)
		{
			this->queue.Enqueue(e);
		}

		template<std::derived_from<ImmediateEvent> T>
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

