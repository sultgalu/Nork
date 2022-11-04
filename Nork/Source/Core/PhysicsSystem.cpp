#include "PhysicsSystem.h"
#include "Components/Physics.h"

namespace Nork {
	void PhysicsSystem::Download(entt::registry& reg)
	{
		using namespace Components;
		uint32_t i = 0;

		reg.view<Transform, Components::Physics>()
			.each([&](entt::entity id, Transform& tr, Components::Physics& phx)
				{
					tr.localPosition += phx.Kinem().position - tr.Position();
					tr.localQuaternion += phx.Kinem().quaternion - tr.Quaternion();
				});
	}
	void PhysicsSystem::Upload(entt::registry& reg, bool updatePoliesForPhysics)
	{
		using namespace Components;

		reg.view<Components::Transform, Components::Physics>()
			.each([&](entt::entity id, Transform& tr, Components::Physics& phx)
				{
					phx.handle.Get().SetColliderSize(tr.Scale());
					phx.Kinem().position = tr.Position();
					phx.Kinem().quaternion = tr.Quaternion();
				});
	}

	void PhysicsSystem::Update(entt::registry& reg)
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