#include "PhysicsSystem.h"
#include "Components/Physics.h"
#include "Scene/Scene.h"

namespace Nork {
static entt::registry& Registry() {
	return Scene::Current().registry;
}
PhysicsSystem::PhysicsSystem()
{
	// these are all the cases when we need to update Physics by Transform
	// all the entities that have Physics and updated Transform (changed by a script or the editor)
	// all the entities that would have entered the <Transform,Physics> group if it existed (initialize Physics by Transform)
	transformObserver.connect(Registry(), entt::collector.update<Components::Transform>().where<Components::Physics>()
		.group<Components::Physics, Components::Transform>());
}
void PhysicsSystem::Download()
{
	using namespace Components;
	uint32_t i = 0;

	Registry().group<Components::Physics, Components::Transform>()
		.each([&](entt::entity id, Components::Physics& phx, Components::Transform& tra)
	{
		bool posChanged = phx.Kinem().position != tra.Position();
		bool quatChanged = phx.Kinem().quaternion != tra.Quaternion();
		if (posChanged || quatChanged)
		{
			Registry().patch<Components::Transform>(id, [&](Components::Transform& tr)
			{
				if (posChanged)
					tr.localPosition += phx.Kinem().position - tr.Position();
				if (quatChanged)
					tr.localQuaternion += phx.Kinem().quaternion - tr.Quaternion();
			});
		}
	});
	// reg.view<Components::Physics>()
	// 	.each([&](entt::entity id, Components::Physics& phx)
	// 		{
	// 			if (phx.Object().TransformChanged())
	// 			{
	// 				reg.patch<Components::Transform>(id, [&](Components::Transform& tr)
	// 					{
	// 						tr.localPosition += phx.Kinem().position - tr.Position();
	// 						tr.localQuaternion += phx.Kinem().quaternion - tr.Quaternion();
	// 					});
	// 			}
	// 		});
	transformObserver.clear();
}
void PhysicsSystem::Upload()
{
	using namespace Components;

	for (auto ent : transformObserver)
	{
		auto& tr = Registry().get<Components::Transform>(ent);
		auto& phx = Registry().get<Components::Physics>(ent);
		phx.handle.Get().size = tr.Scale();
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