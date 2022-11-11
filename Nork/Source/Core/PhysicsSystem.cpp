#include "PhysicsSystem.h"
#include "Components/Physics.h"

namespace Nork {
	PhysicsSystem::PhysicsSystem(entt::registry& reg)
		: reg(reg)
	{
		// these are all the cases when we need to update Physics by Transform
		// all the entities that have Physics and updated Transform (changed by a script or the editor)
		// all the entities that would have entered the <Transform,Physics> group if it existed (initialize Physics by Transform)
		transformObserver.connect(reg, entt::collector.update<Components::Transform>().where<Components::Physics>()
			.group<Components::Transform, Components::Physics>());
	}
	void PhysicsSystem::Download()
	{
		using namespace Components;
		uint32_t i = 0;

		reg.view<Components::Physics>()
			.each([&](entt::entity id, Components::Physics& phx)
				{
					if (phx.Object().TransformChanged())
					{
						reg.patch<Components::Transform>(id, [&](Components::Transform& tr)
							{
								tr.localPosition += phx.Kinem().position - tr.Position();
								tr.localQuaternion += phx.Kinem().quaternion - tr.Quaternion();
							});
					}
				});
		transformObserver.clear();
	}
	void PhysicsSystem::Upload()
	{
		using namespace Components;

		for (auto ent : transformObserver)
		{
			auto& tr = reg.get<Components::Transform>(ent);
			auto& phx = reg.get<Components::Physics>(ent);
			phx.handle.Get().SetColliderSize(tr.Scale());
			phx.Kinem().position = tr.Position();
			phx.Kinem().quaternion = tr.Quaternion();
		}
		transformObserver.clear();
	}

	void PhysicsSystem::Update()
	{
		static Timer deltaTimer(-20);
		float delta = deltaTimer.ElapsedSeconds();
		deltaTimer.Restart();
		if (delta > 0.2f)
			return;

		delta *= physicsSpeed;
		pipeline.Update(delta);
	}

}