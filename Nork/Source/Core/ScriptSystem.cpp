#include "pch.h"
#include "ScriptSystem.h"
#include "App/Application.h"

namespace Nork {
	void ScriptSystem::Update()
	{
		auto& engine = Application::Get().engine;
		
		static auto player = engine.scene.CreateNode()->GetEntity();
		static auto& playerTr = player.AddComponent<Components::Transform>();
		static auto& dr = player.AddComponent<Components::Drawable>();
		static auto& playerKin = player.AddComponent<Components::Physics>().Kinem();
		static auto& coll = player.AddComponent<Components::Collider>() = Components::Collider::Cube();
		static auto& cam = player.AddComponent<Components::Camera>();
		static auto& playerPl = player.AddComponent<Components::PointLight>();
		
		auto setup = [&]()
		{
			playerTr.localPosition = { 0, 0, -15 };
			playerKin.mass = 0.001f;
			engine.physicsSystem.pipeline.coefficient = 0.2f;
			playerPl.SetIntensity(200);
			playerPl.light->color = glm::vec4(1.0f, 0.4f, 0.8f, 1.0f);
			Application::Get().engine.window.Resize(1920, 1080);
			// if (engine.renderingSystem.viewports.empty())
			//  	engine.renderingSystem.viewports.push_back();
			return true;
		};
		static bool set = setup();
		static float speed = 10.0f;
		static float jump = 10.0f;
		//editor.Update();
		auto front = cam.front;
		front.y = 0;
		front = glm::normalize(front);
		auto& input = Application::Get().engine.window.Input();
		glm::vec3 translation = { 0, 0, 0 };
		if (input.IsDown(Key::A))
			translation -= cam.right;
		if (input.IsDown(Key::W))
			translation += front;
		if (input.IsDown(Key::S))
			translation -= front;
		if (input.IsDown(Key::D))
			translation += cam.right;
		if (input.IsDown(Key::Esc))
			Application::Get().engine.window.Close();
		if (input.IsDown(Key::Down) && speed > 0)
			speed /= 1.1f;
		if (input.IsDown(Key::Up))
			speed *= 1.1f;
		if (input.IsDown(Key::Space))
			playerKin.velocity.y = jump;

		//playerTr.Translate(translation * speed);
		playerKin.velocity.x = translation.x * speed;
		playerKin.velocity.z = translation.z * speed;
		cam.position = playerTr.Position();
		if (input.DidScroll())
		{
			cam.Zoom(input.ScrollOffs());
		}

		if (true || input.IsDown(Button::Left))
		{
			cam.Rotate(-input.CursorYOffs(), input.CursorXOffs());
			playerTr.localQuaternion = glm::quat();
			playerTr.Rotate(cam.up, cam.yaw);
			playerTr.Rotate(cam.right, cam.pitch);

			double cy = cos(glm::radians(cam.yaw) * 0.5);
			double sy = sin(glm::radians(cam.yaw) * 0.5);
			double cp = cos(glm::radians(cam.pitch) * 0.5);
			double sp = sin(glm::radians(cam.pitch) * 0.5);
			double cr = cos(0 * 0.5);
			double sr = sin(0 * 0.5);

			glm::quat q;
			q.w = cr * cp * cy + sr * sp * sy;
			q.x = sr * cp * cy - cr * sp * sy;
			q.y = cr * sp * cy + sr * cp * sy;
			q.z = cr * cp * sy - sr * sp * cy;

			playerTr.localQuaternion = q;
		}
		*engine.renderingSystem.viewports[0]->camera = cam;
	}
}
