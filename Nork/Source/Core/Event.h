#pragma once

#include "Platform/Input.h"

namespace Nork
{
	namespace Event
	{
		namespace Types
		{
			using namespace Template::Types;
			template<typename T>
			using Of = MetaInherited<T>;

			struct Base : Of<VirtualType>
			{
				std::source_location from;

				inline virtual size_t GetHash() const
				{	
					return typeid(*this).hash_code();
				}

				template<std::derived_from<Base> T>
				inline static size_t HashOf()
				{
					return typeid(T).hash_code();
				}

				template<std::derived_from<Base> T>
				inline const bool IsType() const { return HashOf<T>() == GetHash(); }

				template<std::derived_from<Base> T>
				inline const constexpr T& As() const { return static_cast<const T&>(*this); }
			};

			// These events get dispatched when events get polled.
			struct InputEvent : Of<Base> {};

			// Contrary to basic Event types, these get dispatched immediately, bypassing the event queue.
			struct AppEvent : Of<Base> {};

			struct KeyEvent : Of<InputEvent>
			{
				KeyEvent(Input::KeyType key) : key(key) {}
				inline auto AsInt() const { return Input::Types::ToInt(key); }
				const Input::KeyType key;
			};

			struct KeyDown : Of<KeyEvent>
			{
				KeyDown(Input::KeyType key) : Of<KeyEvent>(key) {}
			};

			struct KeyUp : Of<KeyEvent>
			{
				KeyUp(Input::KeyType key) : Of<KeyEvent>(key) {}
			};

			struct Type : Of<InputEvent>
			{
				Type(unsigned int c) : character(c) {}
				const unsigned int character;
			};

			struct CursorLeftWindow : Of<InputEvent> {};
			struct CursorEnteredWindow : Of<InputEvent> {};

			struct MonitorConnect : Of<InputEvent> {};
			struct MonitorDisconnect : Of<InputEvent> {};

			struct MouseEvent : Of<InputEvent> {};
			struct MouseButtonEvent : Of<MouseEvent>
			{
				MouseButtonEvent(Input::MouseButtonType button) : button(button) {}
				inline auto AsInt() const { return Input::Types::ToInt(button); }
				Input::MouseButtonType button;
			};

			struct MouseDown : Of<MouseButtonEvent>
			{
				MouseDown(Input::MouseButtonType button) : Of<MouseButtonEvent>(button) {}
			};

			struct MouseUp : Of<MouseButtonEvent>
			{
				MouseUp(Input::MouseButtonType button) : Of<MouseButtonEvent>(button) {}
			};

			struct MouseMove : Of<MouseEvent>
			{
				MouseMove(double offsetX, double offsetY) : offsetX(offsetX), offsetY(offsetY) {}
				double offsetX, offsetY;
			};

			struct MouseScroll : Of<InputEvent>
			{
				MouseScroll(double offset) : offset(offset) {}
				double offset;
			};

			struct WindowInFocus : Of<InputEvent> {};
			struct WindowOutOfFocus : Of<InputEvent> {};

			struct WindowResize : Of<InputEvent>
			{
				WindowResize(int width, int height) : width(width), height(height) {}
				int width, height;
			};

			struct WindowClose : Of<InputEvent>
			{
				WindowClose(bool calledByWindow) : calledByWindow(calledByWindow) {}
				bool calledByWindow;
			};

			struct OnUpdate : Of<AppEvent> {};
			struct Updated : Of<AppEvent> {};
			struct OnRenderUpdate : Of<AppEvent> {};
			struct RenderUpdated : Of<AppEvent> {};
			struct OnPhysicsUpdate : Of<AppEvent> {};
			struct PhysicsUpdated : Of<AppEvent> {};
		}
		
		struct Function
		{
			template<class T>
			using Signature = std::function<void(const T&)>;
			using BaseType = Types::Base;

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

		class Sender: public Registry
		{
		public:
			template<std::derived_from<Types::Base> T>
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

		class Receiver: public Registry
		{
		public:
			template<std::derived_from<Types::Base> T>
			void Subscribe(std::function<void(const T&)> callback)
			{
				this->subscribers[Types::Base::HashOf<T>()].push_back(callback);
			}

			template<std::derived_from<Types::Base> T, typename F, typename M>
			void Subscribe(F func, M member)
			{
				std::function<void(const T&)> cb(std::bind(func, member, std::placeholders::_1));
				this->subscribers[Types::Base::HashOf<T>()].push_back(cb);
			}

			template<std::derived_from<Types::Base> T = Types::Base>
			Receiver Map(std::function<bool(void)> predicate = nullptr)
			{
				return Receiver(*this, T(), predicate);
			}

		private:
			template<std::derived_from<Types::Base> T>
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

	/*class EventQueue
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
	};*/
}

