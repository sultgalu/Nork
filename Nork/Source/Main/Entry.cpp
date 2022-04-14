#include "pch.h"
#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Components/Common.h"
#include "Editor2/Editor.h"
#include "Core/NorkWindow.h"
#include "Core/Engine.h"
#include "App/Application.h"
using namespace Nork;

int main()
{
	Logger::PushStream(std::cout);

	auto& engine = Application::Get().engine;

	Editor2::Editor editor(engine);

	//editor.SetDisplayTexture(engine.renderingSystem.deferredPipeline.lightFb->Color());

	int dim = 8;
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
				ent.AddComponent<Components::Transform>().SetPosition(glm::vec3(i * sep, j * sep, k * sep));
				ent.AddComponent<Components::Kinematic>().mass = 0.1f;
				ent.AddComponent<Components::Collider>() = Components::Collider::Cube();
				ent.AddComponent<Components::Tag>().tag = std::to_string(i).append("-").append(std::to_string(j)).append("-").append(std::to_string(k));
			}
		}
	}

	if (dim > 5)
	{
		engine.physicsSystem.drawPolies = false;
	}

	glm::vec3 scale = glm::vec3(100, 1, 100);
	auto ground = engine.scene.CreateNode()->GetEntity();
	ground.AddComponent<Components::Drawable>().model->meshes[0].material = engine.resourceManager.GetMaterial("a");;
	auto& tr = ground.AddComponent<Components::Transform>()
		.SetPosition(glm::vec3(0, -10, 0))
		.SetScale(scale);
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
	auto& l = sun.AddComponent<Components::DirLight>();
	//l.light->color = glm::vec4(0.5f, 0.4f, 0.25f, 1);
	//l.SetColor(glm::vec4(0.0f));
	sun.AddComponent<Components::DirShadowRequest>();
	l.far = 100; l.near = -100; l.left = -100; l.right = 100; l.bottom = -100; l.top = 100;
	l.RecalcVP(l.GetView());
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
				pl.AddComponent<Components::Transform>().SetPosition({ i * sepP + offsX, j * sepP, k * sepP });
				pl.AddComponent<Components::Drawable>();
				pl.AddComponent<Components::PointLight>().SetIntensity(10);
				//if (shadows-- > 0)
					pl.AddComponent<Components::PointShadowRequest>();
				pl.AddComponent<Components::Tag>().tag = "pointLight" + std::to_string((k-startP) + j * dimP + (i-startP) * dimP * dimP);
			}
		}
	}

	Application::Get().window.SetupCallbacks();
	engine.physicsUpdate = false;
	engine.scene.GetMainCamera().SetPosition(glm::vec3(-58, 16, -91));
	engine.scene.GetMainCamera().SetRotation(-10, 60);
	//engine.Launch();
	Timer t;
	CameraController camContr;

	//engine.StartPhysics();
	while (!Application::Get().window.ShouldClose())
	{
		editor.Update();
		// auto delta = t.Reset();
		// engine.Update();
		// auto& ctx = Application::Get().window.Underlying().GetContext();
		// engine.renderingSystem.DrawToScreen(ctx.width, ctx.height);
	}
}