#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Components/Common.h"
#include "Editor/Editor.h"
#include "Core/Engine.h"
#include "App/Application.h"
using namespace Nork;

int main()
{
	Logger::PushStream(std::cout);

	auto window = Renderer::Vulkan::Window(1920 * 0.8f, 1080 * 0.8f);
	std::unique_ptr<Nork::Input> input = std::make_unique<Nork::Input>(window.glfwWindow);
	Engine engine;

	Renderer::Commands::Instance().BeginTransferCommandBuffer();
	Editor::Editor editor; // needs to record to transfer command buffer
	//editor.SetDisplayTexture(engine.renderingSystem.deferredPipeline.lightFb->Color());

	/*int dim = 10;
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
				ent.AddComponent<Components::Physics>().Kinem().mass = 0.1f;
				ent.AddComponent<Components::Tag>().tag = std::to_string(i).append("-").append(std::to_string(j)).append("-").append(std::to_string(k));
			}
		}
	}*/

	auto pl = engine.scene.CreateNode()->GetEntity();
	pl.AddComponent<Components::Transform>([&](auto& tr)
	{
		tr.position.y = 5;
	}); //{ tr.localPosition = { -8.0, -8.6, -4.7 }; tr.localScale = glm::vec3(0.1f); });
	pl.AddComponent<Components::Drawable>();
	pl.AddComponent<Components::PointLight>([&](Components::PointLight& l)
	{
		l.Data()->color *= 100.0f;
	});
	pl.AddComponent<Components::PointShadowMap>();
	constexpr int levels = 4; //20;
	constexpr int size = 4; // * 4 = one level
	float sep = 2.1f;
	constexpr float height = -10;
	int start = -size / 2;
	constexpr auto allCount = levels * 4 * size;

	auto add = [&](int i, int j, int k)
	{
		auto ent = engine.scene.CreateNode()->GetEntity();
		ent.AddComponent<Components::Transform>([&](auto& tr) { tr.localPosition = glm::vec3(i * sep, height + 2 + j * sep, k * sep); });
		ent.AddComponent<Components::Drawable>(); //.model->meshes[0].material = MaterialResources::Instance().GetById("a");
		ent.AddComponent<Components::Physics>().Kinem().applyGravity = true;
		ent.AddComponent<Components::Tag>().tag = std::to_string(i).append("-").append(std::to_string(j)).append("-").append(std::to_string(k));
	};

	auto bullet = engine.scene.CreateNode()->GetEntity();
	bullet.AddComponent<Components::Drawable>(); // .model->meshes[0].material = MaterialResources::Instance().GetById("a");
	bullet.AddComponent<Components::Transform>([&](auto& tr)
	{
		tr.localPosition = glm::vec3(start - 30, levels * sep / 2 + 10, 0);
		tr.localScale = glm::vec3(1, 15, 5);
		tr.UpdateGlobalWithoutParent();
	});
	bullet.AddComponent<Components::Physics>([&](Components::Physics& phx)
	{
		auto& kinem = phx.Kinem();
		kinem.mass = 1000.0f;
		kinem.velocity.x = 20.0f;
		// kinem.applyGravity = false;
		kinem.w.z = -1.0f;
	});
	bullet.AddComponent<Components::Tag>().tag = "bullet";

	for (int level = 0; level < levels; level++)
	{
		int i = -1, j = 0;
		for (size_t _ = 0; _ < size; _++)
			add(start + ++i, level, start + j);
		for (size_t _ = 0; _ < size; _++)
			add(start + i, level, start + ++j);
		for (size_t _ = 0; _ < size; _++)
			add(start + --i, level, start + j);
		for (size_t _ = 0; _ < size; _++)
			add(start + i, level, start + --j);
	}

	// SINGLE
	// auto ent = engine.scene.CreateNode()->GetEntity();
	// ent.AddComponent<Components::Drawable>().model->meshes[0].material = engine.resourceManager.GetMaterial("a");
	// ent.AddComponent<Components::Transform>([&](Components::Transform& tr)
	// 	{
	// 		tr.position = glm::vec3(0);
	// 		tr.Rotate(glm::vec3(0, 0, 1), -30);
	// 	});
	// ent.AddComponent<Components::Physics>([&](Components::Physics& phx)
	// 	{
	// 		phx.Kinem().mass = 100000.0f;
	// 		// phx.Kinem().mass = 1.0f;
	// 		phx.Kinem().w.z = 10.0f;
	// 	});
	// ent.AddComponent<Components::Collider>() = Components::Collider::Cube();
	// ent.AddComponent<Components::Tag>().tag = "SINGLE";

	// auto ent2 = engine.scene.CreateNode()->GetEntity();
	// ent2.AddComponent<Components::Drawable>().model->meshes[0].material = engine.resourceManager.GetMaterial("a");
	// ent2.AddComponent<Components::Transform>([&](auto& tr) { tr.position = glm::vec3(0, 2, 0.5f); });
	// ent2.AddComponent<Components::Physics>().Kinem().mass = 0.1f;
	// ent2.AddComponent<Components::Collider>() = Components::Collider::Cube();
	// ent2.AddComponent<Components::Tag>().tag = "SINGLE2";

	glm::vec3 scale = glm::vec3(100, 1, 100);
	auto ground = engine.scene.CreateNode()->GetEntity();
	ground.AddComponent<Components::Drawable>(); // .model->meshes[0].material = MaterialResources::Instance().GetById("a");
	ground.AddComponent<Components::Transform>([&](auto& tr)
	{
		tr.localPosition = glm::vec3(0, -10, 0);
	tr.localScale = scale;
	tr.UpdateGlobalWithoutParent();
	});
	ground.AddComponent<Components::Physics>([&](Components::Physics& phx)
	{
		phx.Kinem().isStatic = true;
	phx.Kinem().mass = 1.0f;
	});
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
	sun.AddComponent<Components::DirLight>([](Components::DirLight& l)
	{
		auto writer = l.Data();
		writer->direction = { -0.05f, -0.08f, -0.05f };
		writer->color = glm::vec4(1, 1, 1, 1);
		l.position = { 0, 0, -100 };
		l.rectangle = { 200, 200, 200 };
		l.sun = true;
	});
	sun.AddComponent<Components::DirShadowMap>();
	sun.AddComponent<Components::Tag>().tag = "SUN";

	//auto sun2 = engine.scene.CreateNode()->GetEntity();
	// sun2.AddComponent<Components::DirLight>([](Components::DirLight& l)
	// 	{
	// 		l.light->direction = { 0.05f, -0.08f, 0.05f };
	// 		l.light->color = glm::vec4(1, 1, 1, 1.0f);
	// 		l.position = { 0, 0, -100 };
	// 		l.rectangle = { 200, 200, 200 };
	// 	});
	//l.light->color = glm::vec4(0.5f, 0.4f, 0.25f, 1);
	//l.SetColor(glm::vec4(0.0f));

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
				pl.AddComponent<Components::PointShadowMap>();
				pl.AddComponent<Components::Tag>().tag = "pointLight" + std::to_string((k - startP) + j * dimP + (i - startP) * dimP * dimP);
			}
		}
	}

	Renderer::Vulkan::Window::Instance().SetupCallbacks();
	engine.physicsUpdate = false;
	//engine.Launch();
	Timer t;

	// glfwSetInputMode(Application::Get().engine.window.Underlying().GetContext().glfwWinPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// engine.renderingSystem.viewports.push_back(std::make_shared<Viewport>(std::make_shared<Components::Camera>()));
	// engine.StartPhysics();
	// engine.scene.Load("C:/Users/Norbi/source/repos/Nork/Nork/Assets/scenes/coll.json");

	Renderer::Commands::Instance().EndTransferCommandBuffer();
	Renderer::Commands::Instance().SubmitTransferCommands();
	while (!Renderer::Vulkan::Window::Instance().ShouldClose())
	{
		Input::Instance().Update();
		engine.Update();
		//auto delta = t.Reset();
		//engine.Update();
		//auto& ctx = Application::Get().engine.window.Underlying().GetContext();
		//engine.renderingSystem.DrawToScreen(ctx.width, ctx.height);
	}
	if (engine.physicsUpdate)
	{
		engine.StopPhysics();
	}
	Renderer::Vulkan::Device::Instance().waitIdle();

	editor.BeforeEngineShutdown();
	return 0;
}