#pragma once

#include "Utils/Input.h"

namespace Nork
{
	using namespace Template::Types;
	template<typename T>
	using Of = MetaInherited<T>;

	struct BaseEvent : Of<VirtualType>
	{
		std::source_location from;

		inline virtual size_t GetHash() const
		{
			return typeid(*this).hash_code();
		}

		template<std::derived_from<BaseEvent> T>
		inline static size_t HashOf()
		{
			return typeid(T).hash_code();
		}

		template<std::derived_from<BaseEvent> T>
		inline const bool IsType() const { return HashOf<T>() == GetHash(); }

		template<std::derived_from<BaseEvent> T>
		inline const constexpr T& As() const { return static_cast<const T&>(*this); }
	};

	// These events get dispatched when events get polled.
	struct InputEvent : Of<BaseEvent> {};

	// Contrary to basic Event types, these get dispatched immediately, bypassing the event queue.
	struct AppEvent : Of<BaseEvent> {};

	struct KeyEvent : Of<InputEvent>
	{
		KeyEvent(Key key) : key(key) {}
		inline auto AsInt() const { return ToInt(key); }
		const Key key;
	};

	struct KeyDownEvent : Of<KeyEvent>
	{
		KeyDownEvent(Key key) : Of<KeyEvent>(key) {}
	};

	struct KeyUpEvent : Of<KeyEvent>
	{
		KeyUpEvent(Key key) : Of<KeyEvent>(key) {}
	};

	struct TypeEvent : Of<InputEvent>
	{
		TypeEvent(unsigned int c) : character(c) {}
		const unsigned int character;
	};

	struct CursorLeftWindowEvent : Of<InputEvent> {};
	struct CursorEnteredWindowEvent : Of<InputEvent> {};

	struct MonitorConnectEvent : Of<InputEvent> {};
	struct MonitorDisconnectEvent : Of<InputEvent> {};

	struct MouseEvent : Of<InputEvent> {};
	struct MouseButtonEvent : Of<MouseEvent>
	{
		MouseButtonEvent(MouseButton button) : button(button) {}
		inline auto AsInt() const { return ToInt(button); }
		MouseButton button;
	};

	struct MouseDownEvent : Of<MouseButtonEvent>
	{
		MouseDownEvent(MouseButton button) : Of<MouseButtonEvent>(button) {}
	};

	struct MouseUpEvent : Of<MouseButtonEvent>
	{
		MouseUpEvent(MouseButton button) : Of<MouseButtonEvent>(button) {}
	};

	struct MouseMoveEvent : Of<MouseEvent>
	{
		MouseMoveEvent(double offsetX, double offsetY) : offsetX(offsetX), offsetY(offsetY) {}
		double offsetX, offsetY;
	};

	struct MouseScrollEvent : Of<InputEvent>
	{
		MouseScrollEvent(double offset) : offset(offset) {}
		double offset;
	};

	struct WindowInFocusEvent : Of<InputEvent> {};
	struct WindowOutOfFocusEvent : Of<InputEvent> {};

	struct WindowResizeEvent : Of<InputEvent>
	{
		WindowResizeEvent(int width, int height) : width(width), height(height) {}
		int width, height;
	};

	struct WindowCloseEvent : Of<InputEvent>
	{
		WindowCloseEvent(bool calledByWindow) : calledByWindow(calledByWindow) {}
		bool calledByWindow;
	};

	struct UpdateEvent : Of<AppEvent> {};
	struct UpdatedEvent : Of<AppEvent> {};
	struct RenderUpdateEvent : Of<AppEvent> {};
	struct RenderUpdatedEvent : Of<AppEvent> {};
	struct PhysicsUpdateEvent : Of<AppEvent> {};
	struct PhysicsUpdatedEvent : Of<AppEvent> {};

	struct IdQueryResultEvent : Of<AppEvent>
	{
		IdQueryResultEvent(int x, int y, uint32_t result) :
			x(x), y(y), id(result)
		{
		}
		int x, y;
		uint32_t id;
	};

	struct Function
	{
		template<class T>
		using Signature = std::function<void(const T&)>;
		using BaseType = BaseEvent;

		template<std::derived_from<BaseType> T>
		Function(Signature<T> f)
			: func(*((Signature<BaseType>*)& f))
		{
		}

		template<std::derived_from<BaseType> T>
		void operator()(const T& arg)
		{
			static_cast<Signature<T>>(func)(arg);
		}

		Signature<BaseType> func;
	};

	class Registry : Template::Types::OnlyConstruct
	{
	protected:
		std::unordered_map<size_t, std::vector<Function>> subscribers;
	};

	class Sender : public Registry
	{
	public:
		template<std::derived_from<BaseEvent> T>
		void Send(const T& ev, std::source_location loc = std::source_location::current())
		{
			const_cast<T&>(ev).from = loc;
			auto names = ev.GetNames(); // DEBUG
			auto types = ev.GetIds();
			for (size_t i = 0; i < types.size(); i++)
			{
				auto& type = types[i];

				for (auto& cb : subscribers[type])
				{
					cb(ev);
				}
			}
		}
	};

	class Receiver : public Registry
	{
	public:
		template<std::derived_from<BaseEvent> T>
		void Subscribe(std::function<void(const T&)> callback)
		{
			this->subscribers[BaseEvent::HashOf<T>()].push_back(callback);
		}

		template<std::derived_from<BaseEvent> T, typename F, typename M>
		void Subscribe(F func, M member)
		{
			std::function<void(const T&)> cb(std::bind(func, member, std::placeholders::_1));
			this->subscribers[BaseEvent::HashOf<T>()].push_back(cb);
		}

		template<std::derived_from<BaseEvent> T = BaseEvent>
		Receiver Map(std::function<bool(void)> predicate = nullptr)
		{
			return Receiver(*this, T(), predicate);
		}

	private:
		template<std::derived_from<BaseEvent> T>
		Receiver(Receiver& rec, T t, std::function<bool(void)> predicate)
		{
			if (predicate != nullptr)
			{
				rec.Subscribe(std::function([this, &rec, predicate](const T& ev)
					{
						if (predicate())
						{
							static_cast<Sender&>(static_cast<Registry&>(*this)).Send(ev, ev.from);
						}
					}));
			}
			else
			{
				rec.Subscribe(std::function([&](const T& ev)
					{
						static_cast<Sender&>(static_cast<Registry&>(*this)).Send(ev, ev.from);
					}));
			}
		}
	};

	class Dispatcher : private Registry
	{
	public:
		Sender& GetSender();
		Receiver& GetReceiver();
	};

	inline Sender& Dispatcher::GetSender()
	{
		return static_cast<Sender&>(static_cast<Registry&>(*this));
	}

	inline Receiver& Dispatcher::GetReceiver()
	{
		return static_cast<Receiver&>(static_cast<Registry&>(*this));
	}
}

