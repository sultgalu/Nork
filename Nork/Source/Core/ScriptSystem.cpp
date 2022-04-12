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
		static auto& playerKin = player.AddComponent<Components::Kinematic>();
		static auto& coll = player.AddComponent<Components::Collider>() = Components::Collider::Cube();
		static auto& cam = player.AddComponent<Components::Camera>();
		static auto& playerPl = player.AddComponent<Components::PointLight>();

		auto setup = [&]()
		{
			engine.AddCamera(cam);
			glfwSetInputMode(Application::Get().window.Underlying().GetContext().glfwWinPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			
			playerTr.SetPosition({ 0, 0, -15 });
			playerKin.mass = 0.001f;
			engine.physicsSystem.pipeline.coefficient = 0.2f;

			Application::Get().window.Resize(1920, 1080);
			return true;
		};
		static bool set = setup();
		static float speed = 10.0f;
		static float jump = 10.0f;
		//editor.Update();
		auto front = cam.front;
		front.y = 0;
		front = glm::normalize(front);
		auto& input = Application::Get().window.Input();
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
			Application::Get().window.Close();
		if (input.IsDown(Key::Down) && speed > 0)
			speed /= 1.1f;
		if (input.IsDown(Key::Up))
			speed *= 1.1f;
		if (input.IsDown(Key::Space))
			playerKin.velocity.y = jump;

		//playerTr.Translate(translation * speed);
		playerKin.velocity.x = translation.x * speed;
		playerKin.velocity.z = translation.z * speed;
		cam.position = playerTr.GetPosition();
		if (input.DidScroll())
		{
			cam.Zoom(input.ScrollOffs());
		}

		if (true || input.IsDown(Button::Left))
		{
			cam.Rotate(-input.CursorYOffs(), input.CursorXOffs());
			playerTr.SetRotation(glm::quat());
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

			playerTr.SetRotation(q);
		}

		engine.Cameras()[0] = cam;
	}
}
