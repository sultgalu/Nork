#include "pch.h"
#include "PhysicsSystem.h"

namespace Nork {
	void PhysicsSystem::Update(entt::registry& reg)
	{
		static Timer deltaTimer(-20);
		float delta = deltaTimer.ElapsedSeconds();
		deltaTimer.Restart();
		if (delta > 0.2f)
			return;

		delta *= physicsSpeed;
		deltas.clear();
		Timer t;
		using namespace Physics;
		using namespace Components;

		auto view = reg.view<Components::Transform, Kinematic>(entt::exclude_t<Polygon>());
		auto collOnlyView = reg.view<Components::Transform, Polygon>(entt::exclude_t<Kinematic>());
		auto collView = reg.view<Components::Transform, Kinematic, Polygon>();
		deltas.push_back(std::pair("get entt views", t.Reset()));

		pWorld.kinems.clear();

		collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Polygon& poly)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.GetPosition(), .quaternion = tr.GetRotation(),
					.velocity = kin.velocity, .w = kin.w, .mass = kin.mass,
					.isStatic = false, .forces = kin.forces });
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Polygon& poly)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.GetPosition(), .quaternion = tr.GetRotation(),
					.velocity = glm::vec3(0),.w = glm::vec3(0), .mass = 1,
					.isStatic = true, .forces = glm::vec3(0), });
			});

		view.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.GetPosition(), .quaternion = tr.GetRotation(),
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

			pipeline.SetColliders(colls);
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
				translations.push_back(tr.GetPosition());
				quaternions.push_back(tr.GetRotation());
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Polygon& poly)
			{
				translations.push_back(tr.GetPosition());
				quaternions.push_back(tr.GetRotation());
			});
		deltas.push_back(std::pair("fill model buf", t.Reset()));

		pipeline.SetModels(translations, quaternions);
		deltas.push_back(std::pair("psystem.setmodels", t.Reset()));
		pipeline.Update(delta);
		deltas.push_back(std::pair("psystem.update", t.Reset()));

		uint32_t i = 0;

		auto kinView = reg.view<Transform, Kinematic>();
		deltas.push_back(std::pair("getKinView", t.Reset()));
		kinView.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				tr.SetPosition(pWorld.kinems[i].position);
				kin.velocity = pWorld.kinems[i].velocity;
				kin.forces = pWorld.kinems[i].forces;
				tr.SetRotation(pWorld.kinems[i].quaternion);
				kin.w = pWorld.kinems[i++].w;
			});
		deltas.push_back(std::pair("read back psystem results", t.Reset()));
	}

}