#pragma once

#include "../World.h"
#include "../Data/CollisionResult.h"

namespace Nork::Physics
{
	class Collision
	{
	public:
		Collision(World& world, ColliderIndex idx1, ColliderIndex idx2)
			: world(&world), idx1(idx1), idx2(idx2)
		{}
		Collision() = default;
		Collision& operator=(const Collision&) = default;
		void _1NarrowPhase();
		void _2GenerateContactPoints();
		void _3CalculateForces();
		void _4ResolveVelocities();
		void _4ResolveAngularVelocities();
		void _4ResolvePositions();
		void _4ResolveAll();
		World& GetWorld() const { return *world; }
		const Collider& Collider1() const { return GetWorld().objs[idx1.objIdx].colliders[idx1.collIdx].global; }
		const Collider& Collider2() const { return GetWorld().objs[idx2.objIdx].colliders[idx2.collIdx].global; }
		const KinematicData& Kinem1() const { return GetWorld().objs[idx1.objIdx].kinem; }
		const KinematicData& Kinem2() const { return GetWorld().objs[idx2.objIdx].kinem; }
		const Object& Obj1() const { return GetWorld().objs[idx1.objIdx]; }
		const Object& Obj2() const { return GetWorld().objs[idx2.objIdx]; }
	public:
		ColliderIndex idx1, idx2;
		World* world;

		CollisionResult satRes;
		bool isColliding = false;

		std::vector<glm::vec3> contactPoints = {};
		glm::vec3 contactCenter = glm::zero<glm::vec3>();

		glm::vec3 deltaP1 = glm::vec3(0), deltaP2 = glm::vec3(0);
		glm::vec3 deltaV1 = glm::vec3(0), deltaV2 = glm::vec3(0);
		glm::vec3 deltaW1 = glm::vec3(0), deltaW2 = glm::vec3(0);
	};
}

