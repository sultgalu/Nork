#include "pch.h"
#include "Core/Event.h"
#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Modules/ECS/Storage.h"
#include "Components/Common.h"
#include "Editor/Editor.h"
#include "Platform/Windows.h"
#include "Modules/Renderer/Data/Shader.h"
#include "Modules/Renderer/Data/Mesh.h"
#include "Modules/Renderer/Data/Texture.h"
#include "Modules/Renderer/Utils.h"
#include "Modules/Renderer/Resource/ResourceManager.h"
#include "Modules/Renderer/Resource/DefaultResources.h"
#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Pipeline/Deferred.h"
#include "Core/CameraController.h"
#include "Core/Engine.h"

using namespace Nork;
using namespace Events;
using namespace Input;

static Engine* enginePtr;
Engine& GetEngine()
{
	return *enginePtr;
}

int main()
{
	auto conf = EngineConfig().SetResolution(1280, 720);
	Engine engine(conf);
	enginePtr = &engine;
	Editor::Editor editor(engine.window);

	editor.SetDisplayTexture(engine.pipeline.data.lightPass.tex);
	engine.appEventMan.Subscribe<Events::RenderUpdated>([&](const Event& ev)
		{
			editor.Render();
		});

	auto node = engine.scene.CreateNode();
	engine.scene.AddModelComponent(node, "Resources/Models/lamp/untitled.obj");
	auto& tr = engine.scene.AddComponent<Components::Transform>(node);

	engine.appEventMan.Subscribe<Events::OnUpdate>([&](const Event& e)
		{
			using namespace Input;
			
			static constinit float  speed = 0.005;
			if (engine.window.GetInput().IsKeyDown(Key::Up))
			{
				tr.position.y += speed;
			}
			if (engine.window.GetInput().IsKeyDown(Key::Down))
			{
				tr.position.y -= speed;
			}
			if (engine.window.GetInput().IsKeyDown(Key::Right))
			{
				tr.position.x += speed;
			}
			if (engine.window.GetInput().IsKeyDown(Key::Left))
			{
				tr.position.x -= speed;
			}
		});

	engine.Launch();
}