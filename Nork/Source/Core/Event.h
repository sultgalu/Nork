#pragma once

namespace Nork
{
	// For switch statements to work, and for compile-time evaluation of GetType(), use hard coded values in each derived Events instead of typeid() (which evaluates at runtime)

	struct Event
	{
		inline virtual const size_t GetType() const { return typeid(*this).hash_code(); }

		template<std::derived_from<Event> T>
		inline static constexpr size_t TypeOf() { return typeid(T).hash_code(); }

		template<std::derived_from<Event> T>
		inline const bool IsType() const { return typeid(T).hash_code() == GetType(); }
	};

	namespace Events
	{
		struct KeyDown : public Event
		{
			KeyDown(int code) : keyCode(code) {}
			const int keyCode;
		};

		struct KeyUp : public Event
		{
			KeyUp(int code) : keyCode(code) {}
			const int keyCode;
		};

		struct MouseDown : public Event
		{
			MouseDown(int code) : buttonCode(code) {}
			int buttonCode;
		};

		struct MouseUp : public Event
		{
			MouseUp(int code) : buttonCode(code) {}
			int buttonCode;
		};

		struct MouseMove : public Event
		{
			MouseMove(double offsetX, double offsetY) : offsetX(offsetX), offsetY(offsetY) {}
			double offsetX, offsetY;
		};

		struct MouseScroll : public Event
		{
			MouseScroll(double offset) : offset(offset) {}
			double offset;
		};

		struct WindowResize : public Event
		{
			WindowResize(int width, int height) : width(width), height(height) {}
			int width, height;
		};

		struct WindowClose : public Event
		{
			WindowClose(bool calledByWindow) : calledByWindow(calledByWindow) {}
			bool calledByWindow;
		};
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
		template<std::derived_from<Event> T>
		void RaiseEvent(T e)
		{
			this->queue.Enqueue(e);
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

	private:
		EventQueue queue;
		EventDispatcher dispatcher;
	};
}

