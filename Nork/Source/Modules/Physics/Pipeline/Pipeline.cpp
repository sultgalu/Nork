#include "Pipeline.h"
#include "../Config.h"
#include "../Utils/Clip.h"

namespace Nork::Physics
{
	static std::vector<int> counter;
	static void gen(int n)
	{
		std::ranges::generate(counter, [&n]() mutable { return n++; });
	}

	void Pipeline::Update(float delta)
	{
		broadResults = SAP(world).Get();

		if (counter.size() < broadResults.size())
		{
			counter.resize(broadResults.size());
			gen(0);
		}
		collisions.resize(broadResults.size());

		std::for_each_n(std::execution::par, counter.begin(), broadResults.size(), [&](auto i)
			{
				collisions[i] = Collision(world, broadResults[i].first, broadResults[i].second, coefficient);
				collisions[i]._1NarrowPhase();
				collisions[i]._2GenerateContactPoints();
			});

		std::for_each_n(std::execution::seq, counter.begin(), broadResults.size(), [&](auto i)
			{
				collisions[i]._3CalculateForces(); // should be called together with _4 (resolve as soon as calculated)
				collisions[i]._4ResolveAll();
			});

		std::for_each(std::execution::seq, world.kinems.begin(), world.kinems.end(), [&](auto& kinem)
			{
				VelocityUpdate(kinem, delta);
				RotationUpdate(kinem, delta);
			});
	}

	struct Island
	{
		std::unordered_set<int> objects;
		bool isStatic = false; // has a static object
		KinematicData kinem;
	};

	void Pipeline::SetColliders()
	{
		world.colliders.resize(collidersLocal.size());
		if (counter.size() < collidersLocal.size())
		{
			counter.resize(collidersLocal.size());
			gen(0);
		}
		std::for_each_n(std::execution::par, counter.begin(), collidersLocal.size(), [&](auto i)
			{
				auto& colliderLocal = collidersLocal[i];
				world.colliders[i] = Collider(colliderLocal);
				auto& collider = world.colliders[i];

				glm::mat4 rotation = glm::mat4_cast(world.kinems[i].quaternion);

				// colliders[i].center = rotation * glm::vec4(colliders[i].center, 1);
				// colliders[i].center += translate[i];
				for (size_t j = 0; j < collider.verts.size(); j++)
				{
					collider.verts[j] = rotation * glm::vec4(colliderLocal.verts[j], 1);
					collider.verts[j] += world.kinems[i].position;
				}
				for (size_t j = 0; j < collider.faces.size(); j++)
				{
					collider.faces[j].norm = rotation * glm::vec4(colliderLocal.faces[j].norm, 1);
				}

				collider.center = glm::vec3(0);
				for (auto& vert : collider.verts)
				{
					collider.center += vert;
				}
				collider.center /= collider.verts.size();
			});
		// this->colls = std::vector<Collider>(colls.begin(), colls.end());
	}

	void Pipeline::VelocityUpdate(KinematicData& kinem, float delta)
	{
		if (kinem.isStatic) 
			return;

		glm::vec3 acceleration = kinem.forces / kinem.mass;
		auto deltaV = acceleration * delta;
		glm::vec3 translate = kinem.velocity * delta + 0.5f * acceleration * delta * delta;

		kinem.velocity += deltaV;
		if (glm::isnan(kinem.velocity.x))
			Logger::Error("");
		kinem.position += translate;
		kinem.forces = g * kinem.mass * glm::vec3(0, -1, 0);
	}

	void Pipeline::RotationUpdate(KinematicData& kinem, float delta)
	{
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