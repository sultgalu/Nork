#include "pch.h"
#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Components/Common.h"
#include "Editor/Editor.h"
#include "Core/NorkWindow.h"
#include "Core/Engine.h"
#include "App/Application.h"
using namespace Nork;

int main()
{
	Logger::PushStream(std::cout);

	auto& engine = Application::Get().engine;

	Editor::Editor editor(engine);

	//editor.SetDisplayTexture(engine.renderingSystem.deferredPipeline.lightFb->Color());

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
				auto ent = engine.scene.CreateNode()->GetEntity();
				ent.AddComponent<Components::Drawable>().model->meshes[0].material = engine.resourceManager.GetMaterial("a");
				ent.AddComponent<Components::Transform>([&](auto& tr) { tr.localPosition = glm::vec3(i * sep, j * sep, k * sep); });
				ent.AddComponent<Components::Kinematic>().mass = 0.1f;
				ent.AddComponent<Components::Collider>() = Components::Collider::Cube();
				ent.AddComponent<Components::Tag>().tag = std::to_string(i).append("-").append(std::to_string(j)).append("-").append(std::to_string(k));
			}
		}
	}

	// SINGLE
	//   auto ent = engine.scene.CreateNode()->GetEntity();
	//   ent.AddComponent<Components::Drawable>().model->meshes[0].material = engine.resourceManager.GetMaterial("a");
	//   ent.AddComponent<Components::Transform>([&](auto& tr) { tr.position = glm::vec3(0); });
	//   ent.AddComponent<Components::Kinematic>().mass = 0.1f;
	//   ent.AddComponent<Components::Collider>() = Components::Collider::Cube();
	//   ent.AddComponent<Components::Tag>().tag = "SINGLE";
	//   
	//   auto ent2 = engine.scene.CreateNode()->GetEntity();
	//   ent2.AddComponent<Components::Drawable>().model->meshes[0].material = engine.resourceManager.GetMaterial("a");
	//   ent2.AddComponent<Components::Transform>([&](auto& tr) { tr.position = glm::vec3(0, 2, 0.5f); });
	//   ent2.AddComponent<Components::Kinematic>().mass = 0.1f;
	//   ent2.AddComponent<Components::Collider>() = Components::Collider::Cube();
	//   ent2.AddComponent<Components::Tag>().tag = "SINGLE2";

	glm::vec3 scale = glm::vec3(100, 1, 100);
	auto ground = engine.scene.CreateNode()->GetEntity();
	ground.AddComponent<Components::Drawable>().model->meshes[0].material = engine.resourceManager.GetMaterial("a");
	ground.AddComponent<Components::Transform>([&](auto& tr) {
		tr.localPosition = glm::vec3(0, -10, 0);
		tr.localScale = scale;
		});
	ground.AddComponent<Components::Collider>() = Components::Collider::Cube();
	ground.AddComponent<Components::Tag>().tag = "GROUND";

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

	auto sun = engine.scene.CreateNode()->GetEntity();
	sun.AddComponent<Components::DirLight>([](auto& l)
		{
			l.light->direction = { 0.3f, -0.05f, 0.0f };
			l.far = 100; l.near = -100; l.left = -100; l.right = 100; l.bottom = -100; l.top = 100;
			l.RecalcVP();
			l.sun = true;
		});
	//l.light->color = glm::vec4(0.5f, 0.4f, 0.25f, 1);
	//l.SetColor(glm::vec4(0.0f));

	sun.AddComponent<Components::DirShadowRequest>();
	sun.AddComponent<Components::Tag>().tag = "SUN";
	
	int offsX = 10;
	int dimP = 0;
	int sepP = 3;
	int startP = -dimP / 2;
	int endP = dimP / 2 + dimP % 2;
	int shadows = 8;

	for (int i = startP; i < endP; i++)
	{
		for (int j = 0; j < dimP; j++)
		{
			for (int k = startP; k < endP; k++)
			{
				auto pl = engine.scene.CreateNode()->GetEntity();
				pl.AddComponent<Components::Transform>([&](auto& tr) { tr.localPosition = { i * sepP + offsX, j * sepP, k * sepP }; });
				pl.AddComponent<Components::Drawable>();
				pl.AddComponent<Components::PointLight>().SetIntensity(10);
				//if (shadows-- > 0)
					pl.AddComponent<Components::PointShadowRequest>();
				pl.AddComponent<Components::Tag>().tag = "pointLight" + std::to_string((k-startP) + j * dimP + (i-startP) * dimP * dimP);
			}
		}
	}

	Application::Get().engine.window.SetupCallbacks();
	engine.physicsUpdate = false;
	//engine.Launch();
	Timer t;
	CameraController camContr;

	// glfwSetInputMode(Application::Get().engine.window.Underlying().GetContext().glfwWinPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// engine.renderingSystem.viewports.push_back(std::make_shared<Viewport>(std::make_shared<Components::Camera>()));
	// engine.StartPhysics();
	while (!Application::Get().engine.window.ShouldClose())
	{
		editor.Update();
		//auto delta = t.Reset();
		//engine.Update();
		//auto& ctx = Application::Get().engine.window.Underlying().GetContext();
		//engine.renderingSystem.DrawToScreen(ctx.width, ctx.height);
	}
	if (engine.physicsUpdate)
	{
		engine.StopPhysics();
	}

	return 0;
}