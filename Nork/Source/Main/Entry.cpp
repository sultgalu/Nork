#include "pch.h"
#include "Core/Event.h"
#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Components/Common.h"
#include "Editor/Editor.h"
#include "Core/NorkWindow.h"
#include "Modules/Renderer/Data/Shader.h"
#include "Modules/Renderer/Data/Mesh.h"
#include "Modules/Renderer/Data/Texture.h"
#include "Modules/Renderer/Utils.h"
#include "Modules/Renderer/Resource/ResourceCreator.h"
#include "Modules/Renderer/Resource/DefaultResources.h"
#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Pipeline/Deferred.h"
#include "Core/Engine.h"
#include "App/Application.h"
using namespace Nork;

static Engine* enginePtr;
Engine& GetEngine()
{
	return *enginePtr;
}

int main()
{
	Logger::PushStream(std::cout);

	auto& engine = Application::Get().engine;
	auto& dispatcher = Application::Get().dispatcher;

	auto cameraNode = engine.scene.CreateNode();
	auto& cam = engine.scene.AddComponent<Components::Camera>(cameraNode);
	enginePtr = &engine;
	Editor::Editor editor(engine);

	editor.SetDisplayTexture(engine.lightFb.Result());
	dispatcher.GetReceiver().Subscribe<RenderUpdatedEvent>([&](const RenderUpdatedEvent& ev)
		{
			editor.Render();
		});

	int dim = 4;
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
				engine.scene.AddComponent<Components::Kinematic>(node).mass = 0.1f;
				engine.scene.AddComponent<Polygon>(node);
				engine.scene.AddComponent<Components::Tag>(node).tag = std::to_string(i).append("-").append(std::to_string(j)).append("-").append(std::to_string(k));
			}
		}
	}

	if (dim > 5)
	{
		engine.drawPolies = false;
	}

	glm::vec3 scale = glm::vec3(100, 1, 100);
	auto ground = engine.scene.CreateNode();
	engine.scene.AddModel(ground);
	auto& tr = engine.scene.AddComponent<Components::Transform>(ground);
	tr.position = glm::vec3(0, -10, 0);
	tr.scale = scale;
	engine.scene.AddComponent<Polygon>(ground).Scale(scale);
	engine.scene.AddComponent<Components::Tag>(ground).tag = "GROUND";

	/*auto node1 = engine.scene.CreateNode();
	engine.scene.AddModel(node1);
	engine.scene.AddComponent<Components::Transform>(node1).position = glm::vec3(0, 0, 1);
	engine.scene.AddComponent<Components::Kinematic>(node1);
	engine.scene.AddComponent<Poly>(node1);
	engine.scene.AddComponent<Components::Tag>(node1).tag = "NODE 1";

	auto node2 = engine.scene.CreateNode();
	engine.scene.AddModel(node2);
	engine.scene.AddComponent<Components::Transform>(node2).position = glm::vec3(0, 0, -1);
	engine.scene.AddComponent<Components::Kinematic>(node2);
	engine.scene.AddComponent<Poly>(node2);
	engine.scene.AddComponent<Components::Tag>(node2).tag = "NODE 2";*/

	auto sun = engine.scene.CreateNode();
	auto& l = engine.scene.AddComponent<Components::DirLight>(sun);
	l.SetColor(glm::vec4(0.1f, 0.08f, 0.05, 1));
	auto& shad = engine.scene.AddComponent<Components::DirShadow>(sun);
	shad.far = 100; shad.near = -100; shad.left = -100; shad.right = 100; shad.bottom = -100; shad.top = 100;
	shad.RecalcVP(l.GetView());
	engine.scene.AddComponent<Components::Tag>(sun).tag = "SUN";
	
	//int offsX = 10;
	//int dimP = 5;
	//int sepP = 3;
	//int startP = -dimP / 2;
	//int endP = dimP / 2 + dimP % 2;

	//for (int i = startP; i < endP; i++)
	//{
	//	for (int j = 0; j < dimP; j++)
	//	{
	//		for (int k = startP; k < endP; k++)
	//		{
	//			auto node = engine.scene.CreateNode();
	//			engine.scene.AddComponent<Components::PointLight>(node);
	//			engine.scene.AddModel(node);
	//			engine.scene.AddComponent<Components::Transform>(node).position = glm::vec3(i * sepP + offsX, j * sepP, k * sepP);
	//			//engine.scene.AddComponent<Components::Kinematic>(node).mass = 0.1f;
	//			//engine.scene.AddComponent<Polygon>(node);
	//			//engine.scene.AddComponent<Components::Tag>(node).tag = std::to_string(i).append("-").append(std::to_string(j)).append("-").append(std::to_string(k));
	//		}
	//	}
	//}

	/*engine.appEventMan.GetReceiver().Subscribe<Event::Types::OnUpdate>([&](const Event::Types::OnUpdate& e)
		{
			using namespace Components;
			using namespace Input;

		});*/

	Application::Get().window.SetupCallbacks();
	//Application::Get().window.Resize(1280, 720);
	engine.Launch();
}