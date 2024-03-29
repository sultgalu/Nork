#include "Pipeline.h"
#include "../Algorithms/SAP.h"

namespace Nork::Physics
{
	static std::vector<int> counter;
	static void gen(int n)
	{
		std::ranges::generate(counter, [&n]() mutable { return n++; });
	}

	void Pipeline::Update(float delta)
	{
		Timer t;
		std::for_each(std::execution::par_unseq, world.objs.begin(), world.objs.end(), [&](Object& obj)
			{
				obj.OnMassChanged();
				VelocityUpdate(obj, delta);
				RotationUpdate(obj, delta);
				obj.UpdateGlobalColliders(); // only needed when transform changed
			});
		broadResults = SAP(world.objs).Get();

		if (counter.size() < broadResults.size())
		{
			counter.resize(broadResults.size());
			gen(0);
		}
		collisions.resize(broadResults.size());

		std::for_each_n(std::execution::par_unseq, counter.begin(), broadResults.size(), [&](int i)
			{
				collisions[i] = Collision(world, broadResults[i].first, broadResults[i].second);
				collisions[i]._1NarrowPhase();
				collisions[i]._2GenerateContactPoints();
			});

		// works faster with ::par, but in theory data-races can occour
		std::for_each_n(std::execution::unseq, counter.begin(), broadResults.size(), [&](int i)
			{
				collisions[i]._3CalculateForces(); // should be called together with _4 (resolve as soon as calculated)
				collisions[i]._4ResolveAll();
			});
		executionTime = t.Elapsed();
	}

	void Pipeline::VelocityUpdate(Object& obj, float delta)
	{
		auto& kinem = obj.kinem;
		if (kinem.isStatic) 
			return;

		glm::vec3 acceleration = kinem.forces / kinem.mass;
		auto deltaV = acceleration * delta;
		glm::vec3 translate = kinem.velocity * delta + 0.5f * acceleration * delta * delta;

		kinem.velocity += deltaV;
		if (glm::isnan(kinem.velocity.x))
			Logger::Error("");
		kinem.position += translate;
		if (kinem.applyGravity)
			kinem.forces = g * kinem.mass * glm::vec3(0, -1, 0);
	}

	void Pipeline::RotationUpdate(Object& obj, float delta)
	{
		auto& kinem = obj.kinem;
		if (kinem.isStatic || glm::dot(kinem.w, kinem.w) == 0)
			return;

		float angle = glm::length(kinem.w);
		if (angle > 0)
		{
			glm::quat rot(2, kinem.w * delta);
			rot = glm::normalize(rot);
			kinem.quaternion = rot * kinem.quaternion;

			if (glm::isnan(kinem.torque.x))
				Logger::Error("");
			glm::vec3 angularAcc = kinem.torque / kinem.I;
			kinem.w += angularAcc * delta;
			kinem.torque = glm::vec3(0);
		}
	}
}
