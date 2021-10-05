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

	engine.Launch();
}