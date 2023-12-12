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

	// Registry().group<Components::Physics, Components::Transform>()
	// 	.each([&](entt::entity id, Components::Physics& phx, Components::Transform& tra)
	// {
	// 	bool posChanged = phx.Kinem().position != tra.Position();
	// 	bool quatChanged = phx.Kinem().quaternion != tra.Quaternion();
	// 	if (posChanged || quatChanged)
	// 	{
	// 		Registry().patch<Components::Transform>(id, [&](Components::Transform& tr)
	// 		{
	// 			if (posChanged)
	// 				tr.localPosition += phx.Kinem().position - tr.Position();
	// 			if (quatChanged)
	// 				tr.localQuaternion += phx.Kinem().quaternion - tr.Quaternion();
	// 		});
	// 	}
	// });
	auto group = Registry().group<Components::Physics, Components::Transform>();
	std::mutex mutex;
	std::for_each(std::execution::par_unseq, group.begin(), group.end(), [&](entt::entity id)
	{
		auto& phx = group.get<Components::Physics>(id);
		auto& tra = group.get<Components::Transform>(id);
		bool posChanged = phx.Kinem().position != tra.Position();
		bool quatChanged = phx.Kinem().quaternion != tra.Quaternion();

		if (posChanged || quatChanged)
		{
			if (posChanged)
				tra.localPosition += phx.Kinem().position - tra.Position();
			if (quatChanged)
				tra.localQuaternion += phx.Kinem().quaternion - tra.Quaternion();
			// Registry().patch<Components::Transform>(id, [&](Components::Transform& tr)
			// {
			// 	if (posChanged)
			// 		tr.localPosition += phx.Kinem().position - tr.Position();
			// 	if (quatChanged)
			// 		tr.localQuaternion += phx.Kinem().quaternion - tr.Quaternion();
			// });
			const std::lock_guard<std::mutex> lock(mutex); // EnTT is not thread safe
			Registry().patch<Components::Transform>(id); // notify observers
			//Registry().replace<Components::Transform>(id, tra);
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

	std::for_each(std::execution::par_unseq, transformObserver.begin(), transformObserver.end(), [&](entt::entity id)
	{
		auto& tr = Registry().get<Components::Transform>(id);
		auto& phx = Registry().get<Components::Physics>(id);
		phx.handle.Get().size = tr.Scale();
		phx.Kinem().position = tr.Position();
		phx.Kinem().quaternion = tr.Quaternion();
	});
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