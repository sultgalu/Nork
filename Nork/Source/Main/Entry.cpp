#include "pch.h"
#include "Core/Event.h"
#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Modules/ECS/Storage.h"
#include "Components/Common.h"
#include "Editor/Editor.h"
#include "Platform/Window.h"
#include "Modules/Renderer/Data/Shader.h"
#include "Modules/Renderer/Data/Mesh.h"
#include "Modules/Renderer/Data/Texture.h"
#include "Modules/Renderer/Utils.h"
#include "Modules/Renderer/Resource/ResourceCreator.h"
#include "Modules/Renderer/Resource/DefaultResources.h"
#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Pipeline/Deferred.h"
#include "Core/Engine.h"

using namespace Nork;
using namespace Input;

static Engine* enginePtr;
Engine& GetEngine()
{
	return *enginePtr;
}

int main()
{
	Logger::PushStream(std::cout);

	auto conf = EngineConfig().SetResolution(1280, 720);
	Engine engine(conf);
	enginePtr = &engine;
	Editor::Editor editor(engine);

	editor.SetDisplayTexture(engine.lightFb.Result());
	engine.appEventMan.GetReceiver().Subscribe<Event::Types::RenderUpdated>([&](const Event::Types::RenderUpdated& ev)
		{
			editor.Render();
		});

	{
		using namespace Event::Types;
		using enum Input::KeyType;
	}

	/*auto node = engine.scene.CreateNode();
	engine.scene.AddModel(node, "Resources/Models/lamp/untitled.obj");
	engine.scene.AddComponent<Components::Transform>(node);
	auto& pl = engine.scene.AddComponent<Components::PointLight>(node);
	engine.scene.AddComponent<Components::PointShadow>(node);
	auto model = engine.scene.CreateNode();
	engine.scene.AddComponent<Components::Transform>(model).position.x = 2.5f;
	engine.scene.AddModel(model);
	engine.scene.AddComponent<Poly>(node, 0.2f);
	engine.scene.AddComponent<Poly>(model);
	engine.scene.AddComponent<Components::Kinematic>(node);
	engine.scene.AddComponent<Components::Kinematic>(model);*/

	int dim = 2;
	int sep = 3;
	int start = -dim / 2;
	int end = dim / 2 + dim % 2;

	for (int i = start; i < end; i++)
	{
		for (int j = 0; j < dim; j++)
		{
			for (int k = start; k < end; k++)
			{
				auto node = engine.scene.CreateNode();
				engine.scene.AddModel(node);
				engine.scene.AddComponent<Components::Transform>(node).position = glm::vec3(i * sep, j * sep, k * sep);
				engine.scene.AddComponent<Components::Kinematic>(node);
				engine.scene.AddComponent<Poly>(node);
				engine.scene.AddComponent<Components::Tag>(node).tag = std::to_string(i).append("-").append(std::to_string(j)).append("-").append(std::to_string(k));
			}
		}
	}

	glm::vec3 scale = glm::vec3(100, 1, 100);
	auto ground = engine.scene.CreateNode();
	engine.scene.AddModel(ground);
	auto& tr = engine.scene.AddComponent<Components::Transform>(ground);
	tr.position = glm::vec3(0, -10, 0);
	tr.scale = scale;
	engine.scene.AddComponent<Poly>(ground).Scale(scale);
	engine.scene.AddComponent<Components::Tag>(ground).tag = "GROUND";

	auto sun = engine.scene.CreateNode();
	engine.scene.AddComponent<Components::DirLight>(sun).SetColor(glm::vec4(1.0f, 0.8f, 0.5, 1));
	engine.scene.AddComponent<Components::DirShadow>(sun);
	engine.scene.AddComponent<Components::Tag>(sun).tag = "SUN";

	engine.UpdatePoliesForPhysics();
	//test5();

	/*engine.window.GetInputEvents().Subscribe<Event::Types::InputEvent>([](const Event::Types::InputEvent& ev)
		{
			Logger::Debug("Input Event: ", ev.GetName(), " From: ", ev.from.file_name(), ":", ev.from.line());
		});*/

	engine.appEventMan.GetReceiver().Subscribe<Event::Types::OnUpdate>([&](const Event::Types::OnUpdate& e)
		{
			using namespace Input;
			
			/*static constinit float  speed = 0.005;
			if (engine.window.GetInputState().Is(KeyType::Up, KeyState::Down))
			{
				pl.GetMutableData().position.y += speed;
			}
			if (engine.window.GetInputState().Is(KeyType::Down, KeyState::Down))
			{
				pl.GetMutableData().position.y -= speed;
			}
			if (engine.window.GetInputState().Is(KeyType::Right, KeyState::Down))
			{
				pl.GetMutableData().position.x += speed;
			}
			if (engine.window.GetInputState().Is(KeyType::Left, KeyState::Down))
			{
				pl.GetMutableData().position.x -= speed;
			}*/
		});

	engine.Launch();
}