#include "Engine.h"

#include "Modules/Renderer/LoadUtils.h"
#include "Core/InputState.h"
#include "App/Application.h"

namespace Nork
{
	static std::vector<uint8_t> dShadowIndices;
	static std::vector<uint8_t> pShadowIndices;
	static constexpr auto dShadIdxSize = 10;
	static constexpr auto pShadIdxSize = 10;

	void Engine::OnDShadowAdded(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		shad.idx = dShadowIndices.back();
		dShadowIndices.pop_back();

		// should handle it elsewhere
		auto light = reg.try_get<Components::DirLight>(id);
		if (light != nullptr)
			shad.RecalcVP(light->GetView());
	}
	void Engine::OnDShadowRemoved(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		dShadowIndices.push_back(shad.idx);
	}

	Engine::Engine(EngineConfig config) : 
		pSystem()
	{
		auto& reg = scene.registry;
		reg.on_construct<Components::DirShadow>().connect<&Engine::OnDShadowAdded>(this);
		reg.on_destroy<Components::DirShadow>().connect<&Engine::OnDShadowRemoved>(this);
		renderingSystem.Init();

		for (int i = dShadIdxSize - 1; i > -1; i--)
		{
			dShadowIndices.push_back(i);
		}
		for (int i = pShadIdxSize - 1; i > -1; i--)
		{
			pShadowIndices.push_back(i);
		}
	}

	void Engine::Launch()
	{
		Nork::Window& win = Application::Get().window;

		auto& sender = Application::Get().dispatcher.GetSender();
		while (win.IsRunning())
		{	
			sender.Send(UpdateEvent());
			sender.Send(RenderUpdateEvent());
			
			renderingSystem.Update(scene);
			// PhysicsUpdate(); // NEW

			sender.Send(RenderUpdatedEvent());
			Profiler::Clear();
			win.Refresh();

			sender.Send(UpdatedEvent());
		}
	}
	void Engine::ReadId(int x, int y)
	{
		/*uint32_t res;
		Renderer::Utils::Other::ReadPixels(lightFb.GetFBO(), lightFb.ColorAttForExtension(0), x, y, Renderer::Utils::Texture::Format::R32UI, &res);
		if (res != 0)
		{
			Application::Get().dispatcher.GetSender().Send(IdQueryResultEvent(x, y, res));
		}*/
	}
	Engine::~Engine()
	{
	}
	static std::vector<std::pair<std::string, float>> deltas;
	std::vector<std::pair<std::string, float>> Engine::GetDeltas()
	{
		return deltas;
	}
	void Engine::PhysicsUpdate()
	{
		static Timer deltaTimer(-20);
		float delta = deltaTimer.ElapsedSeconds();
		deltaTimer.Restart();
		if (delta > 0.2f)
			return;

		using namespace Physics;
		using namespace Components;

		if (!physicsUpdate) return;
		//delta = 0.001f;
		delta *= physicsSpeed;
		deltas.clear();
		Timer t;

		auto& reg = scene.registry;
		auto view = reg.view<Components::Transform, Kinematic>(entt::exclude_t<Polygon>());
		auto collOnlyView = reg.view<Components::Transform, Polygon>(entt::exclude_t<Kinematic>());
		auto collView = reg.view<Components::Transform, Kinematic, Polygon>();
		deltas.push_back(std::pair("get entt views", t.Reset()));

		pWorld.kinems.clear();

		collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Polygon& poly)
			{
				pWorld.kinems.push_back(Physics::KinematicData{ 
					.position = tr.position, .quaternion = tr.quaternion,
					.velocity = kin.velocity, .w = kin.w, .mass = kin.mass, 
					.isStatic = false, .forces = kin.forces });
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Polygon& poly)
			{
				pWorld.kinems.push_back(Physics::KinematicData{ 
					.position = tr.position, .quaternion = tr.quaternion,
					.velocity = glm::vec3(0),.w = glm::vec3(0), .mass = 1, 
					.isStatic = true, .forces = glm::vec3(0),  });
			});

		view.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.position, .quaternion = tr.quaternion,
					.velocity = kin.velocity, .w = kin.w, .mass = kin.mass, 
					.isStatic = false, .forces = kin.forces });
			});
		deltas.push_back(std::pair("update kinems", t.Reset()));

		static bool first = true;
		if (updatePoliesForPhysics)
		{
			std::vector<Collider> colls;

			collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Polygon& poly)
				{
					colls.push_back(poly.AsCollider());
				});

			collOnlyView.each([&](entt::entity id, Transform& tr, Polygon& poly)
				{
					colls.push_back(poly.AsCollider());
				});

			pSystem.SetColliders(colls);
			updatePoliesForPhysics = false;
		}
		static std::vector<glm::vec3> translations;
		static std::vector<glm::quat> quaternions;
		translations.clear();
		translations.reserve(reg.size<Transform>());
		quaternions.clear();
		quaternions.reserve(reg.size<Transform>());
		deltas.push_back(std::pair("clear model bufs", t.Reset()));

		collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Polygon& poly)
			{
				translations.push_back(tr.position);
				quaternions.push_back(tr.quaternion);
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Polygon& poly)
			{
				translations.push_back(tr.position);
				quaternions.push_back(tr.quaternion);
			});
		deltas.push_back(std::pair("fill model buf", t.Reset()));

		pSystem.SetModels(translations, quaternions);
		deltas.push_back(std::pair("psystem.setmodels", t.Reset()));
		pSystem.Update(delta);
		deltas.push_back(std::pair("psystem.update", t.Reset()));

		uint32_t i = 0;

		auto kinView = reg.view<Transform, Kinematic>();
		deltas.push_back(std::pair("getKinView", t.Reset()));
		kinView.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				tr.position = pWorld.kinems[i].position;
				kin.velocity = pWorld.kinems[i].velocity;
				kin.forces = pWorld.kinems[i].forces;
				tr.quaternion = pWorld.kinems[i].quaternion;
				kin.w = pWorld.kinems[i++].w;
			});
		deltas.push_back(std::pair("read back psystem results", t.Reset()));
	}
}