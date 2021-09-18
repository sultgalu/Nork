#include "pch.h"
#include "Core/Event.h"
#include "Utils/Logger.h"
#include "format"
#include "Utils/Timer.h"
#include "Modules/ECS/Storage.h"
#include "Editor/Editor.h"
#include <Platform/Interface/Windows.h>


using namespace Nork;
using namespace Events;

void EventHandler(const Event& e)
{
	if (e.IsType<KeyDown>())
	{
		Logger::Error.Log("KEYDOWN event, keycode: ", std::to_string(static_cast<const KeyDown&>(e).keyCode).c_str());
	}
	if (e.IsType<KeyUp>())
	{
		Logger::Error.Log("KEYUP event, keycode: ", std::to_string(static_cast<const KeyUp&>(e).keyCode).c_str());
	}
	if (e.IsType<MouseDown>())
	{
		Logger::Error.Log("MOUSEDOWN event, keycode: ", std::to_string(static_cast<const MouseDown&>(e).buttonCode).c_str());
	}
}

#define name_of(T) #T

template<typename T>
struct Holder
{
	T val;
	const char* GetName()
	{
		return name_of(T);
	}
};

int main() 
{
	Timer t;
	
	EventDispatcher ed;
	ed.Subscribe<Event>([](const Event& e) { return; });
	auto key = KeyDown(2);
	ed.Dispatch(key);

	EventQueue q;

	entt::registry reg0;
	auto id = reg0.create();
	reg0.emplace<EventManager>(id);
	auto& man = reg0.get<EventManager>(id);

	q.Enqueue(KeyDown(2));
	Logger::PushStream(std::cout);

	Logger::Info.Log("asd", "aaaa", "123");
	man.Subscribe<KeyDown>([](const Event& e)
		{
			KeyDown ev = static_cast<const KeyDown&>(e);
			Logger::Debug.Log("KeyDown Event: ", std::to_string(ev.keyCode).c_str(), " typeID: ", std::to_string(e.GetType()).c_str());
		});

	man.Subscribe<KeyUp>([](const Event& e)
		{
			KeyUp ev = static_cast<const KeyUp&>(e);
			Logger::Debug.Log("KeyUp Event: ", std::to_string(ev.keyCode).c_str(), " typeID: ", std::to_string(e.GetType()).c_str());
		});

	man.Subscribe<KeyDown>(&EventHandler);
	man.Subscribe<KeyUp>(&EventHandler);
	man.Subscribe<MouseDown>(&EventHandler);

	man.RaiseEvent(KeyDown(4));
	man.RaiseEvent(KeyUp(6));
	man.RaiseEvent(MouseDown(2));

	man.PollEvents();

	Logger::Warning.Log("time elapsed: ", t.Elapsed(), " ms");

	ECS::Registry reg;
	entt::type_list<int, long, char> list;
	ECS::TypeList<int, long, char> list2;

	Logger::Info.Log(sizeof(list), " ", list.size);
	Logger::Info.Log(sizeof(list2), " ", list2.size);
	reg.GetComponents<int, long, char>();
	std::function<void()>fun = []()
	{

	};
	
	auto lambda = [](const entt::entity ent, long& l, char& c)
	{
		Logger::Info.Log(l, ":::::::::", c);
	};
	auto ent = reg.CreateEntity();
	auto& val1 = reg.Emplace<long>(ent);
	auto& val2 = reg.Emplace<char>(ent);
	val1 = 45;
	val2 = 'a';
	
	reg.GetComponents<long, char>().ForEach(lambda);

	float arr[] = { 1,2 };
	Logger::Info.Log(1, "asd", true, 2.3, 2.3f, 'a', 'b', arr);
	Logger::Error.Log(1);
	Logger::Error(2, " asd ", 'c');
	MetaLogger().Debug("This ", "is ", " a msg with num: ", 3);

	Logger::Info(Holder<int>().GetName());

	Window<GLFWwindow> win(1000, 1000);

	Editor::Editor<GLFWwindow> editor(win);

	while (win.IsRunning())
	{
		editor.Render();
		win.Refresh();
		win.GetEventManager().PollEvents();
	}
}