#include "pch.h"
#include "Core/Event.h"
#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Modules/ECS/Storage.h"
#include "Editor/Editor.h"
#include "Platform/Interface/Windows.h"
#include "Modules/Renderer/Data/Shader.h"
#include "Modules/Renderer/Data/Mesh.h"
#include "Modules/Renderer/Data/Texture.h"
#include "Modules/Renderer/Utils.h"
#include "Modules/Renderer/Resource/ResourceManager.h"
#include "Modules/Renderer/Resource/DefaultResources.h"
#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Pipeline/Deferred.h"

using namespace Nork;
using namespace Events;

void EventHandler(const Event& e)
{
	if (e.IsType<KeyDown>())
	{
		Logger::Error.Log("KEYDOWN event, keycode: ", std::to_string(std::to_underlying(static_cast<const KeyDown&>(e).key)).c_str());
	}
	if (e.IsType<KeyUp>())
	{
		Logger::Error.Log("KEYUP event, keycode: ", std::to_string(std::to_underlying(static_cast<const KeyUp&>(e).key)).c_str());
	}
	if (e.IsType<MouseDown>())
	{
		Logger::Error.Log("MOUSEDOWN event, keycode: ", std::to_string(std::to_underlying(static_cast<const MouseDown&>(e).button)).c_str());
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

Renderer::Data::Shader CreateShaderFromPath(std::string_view path)
{
	using namespace Renderer;

	std::ifstream stream(path.data());
	std::stringstream buf;
	buf << stream.rdbuf();

	auto data = Data::ShaderData{ .source = buf.str() };
	auto resource = Resource::CreateShader(data);
	stream.close();
	return Data::Shader(resource);
}
using namespace Input;

int main() 
{
	Timer t;
	
	EventDispatcher ed;
	ed.Subscribe<Event>([](const Event& e) { return; });
	auto key = KeyDown(Key::Up);
	ed.Dispatch(key);

	EventQueue q;

	entt::registry reg0;
	auto id = reg0.create();
	reg0.emplace<EventManager>(id);
	auto& man = reg0.get<EventManager>(id);

	q.Enqueue(KeyDown(Key::Up));
	Logger::PushStream(std::cout);

	Logger::Info.Log("asd", "aaaa", "123");
	man.Subscribe<KeyDown>([](const Event& e)
		{
			KeyDown ev = static_cast<const KeyDown&>(e);
			Logger::Debug.Log("KeyDown Event: ", Input::ToString(ev.key).c_str(), " typeID: ", std::to_string(e.GetType()).c_str());
		});

	man.Subscribe<KeyUp>([](const Event& e)
		{
			KeyUp ev = static_cast<const KeyUp&>(e);
			Logger::Debug.Log("KeyUp Event: ", Input::ToString(ev.key).c_str(), " typeID: ", std::to_string(e.GetType()).c_str());
		});

	man.Subscribe<KeyDown>(&EventHandler);
	man.Subscribe<KeyUp>(&EventHandler);
	man.Subscribe<MouseDown>(&EventHandler);

	man.RaiseEvent(KeyDown(Key::A));
	man.RaiseEvent(KeyUp(Key::W));
	man.RaiseEvent(MouseDown(MouseButton::Left));

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

	Window win(1280, 720);

	Editor::Editor editor(win);
	Renderer::Data::Shader shader = CreateShaderFromPath("Source/Shaders/lightPass.shader");
	shader.Use();

	// MUST
	Renderer::Resource::DefaultResources::Init();
	auto model = Renderer::Loaders::LoadModel("Resources/Models/lamp/untitled.obj");
	std::vector<Renderer::Data::MeshResource> meshResources;
	std::vector<Renderer::Data::Mesh> meshes;
	for (size_t i = 0; i < model.size(); i++)
	{
		meshResources.push_back(Renderer::Resource::CreateMesh(model[i]));
		meshes.push_back(Renderer::Data::Mesh(meshResources[i]));
	}

	Renderer::Pipeline::Deferred pipeline(Renderer::Pipeline::DeferredData (
		Renderer::Pipeline::DeferredData::Shaders
		{
			.gPass = CreateShaderFromPath("Source/Shaders/gPass.shader"),
			.lPass = CreateShaderFromPath("Source/Shaders/lightPass.shader"),
			.skybox = CreateShaderFromPath("Source/Shaders/skybox.shader"),
		}));

	std::vector<Renderer::Data::Model> models;
	std::vector<Renderer::Data::DirLight> dLights;
	std::vector<Renderer::Data::PointLight> pLights;
	models.push_back(Renderer::Data::Model(meshes, glm::identity<glm::mat4>()));

	auto cam = std::make_shared<Components::Camera>();
	
	auto camController = CameraController(win.GetEventManager(), cam);
	editor.SetDisplayTexture(pipeline.data.lightPass.tex);
	while (win.IsRunning())
	{
		win.GetEventManager().RaiseEvent(Events::OnUpdate());
		pipeline.DrawScene(models);
		editor.Render();
		win.Refresh();
		win.GetEventManager().RaiseEvent(Events::Updated());
		win.GetEventManager().PollEvents();
	}

	for (size_t i = 0; i < meshResources.size(); i++)
	{
		Renderer::Resource::DeleteMesh(meshResources[i]);
	}
}