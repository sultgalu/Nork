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

	editor.SetDisplayTexture(engine.pipeline.data.lightPass.tex);
	engine.appEventMan.GetReceiver().Subscribe<Event::Types::RenderUpdated>([&](const Event::Types::RenderUpdated& ev)
		{
			editor.Render();
		});

	{
		using namespace Event::Types;
		using enum Input::KeyType;
	}

	auto node = engine.scene.CreateNode();
	engine.scene.AddModel(node, "Resources/Models/lamp/untitled.obj");
	auto& tr = engine.scene.AddComponent<Components::Transform>(node);
	//auto& dl = engine.scene.AddComponent<Components::DirLight>(node);
	auto& pl = engine.scene.AddComponent<Components::PointLight>(node);
	pl.SetPower(1);

	/*engine.window.GetInputEvents().Subscribe<Event::Types::InputEvent>([](const Event::Types::InputEvent& ev)
		{
			Logger::Debug("Input Event: ", ev.GetName(), " From: ", ev.from.file_name(), ":", ev.from.line());
		});*/

	engine.appEventMan.GetReceiver().Subscribe<Event::Types::OnUpdate>([&](const Event::Types::OnUpdate& e)
		{
			using namespace Input;
			
			static constinit float  speed = 0.005;
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
			}
		});

	engine.Launch();
}