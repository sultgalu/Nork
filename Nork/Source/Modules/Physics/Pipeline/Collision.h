#pragma once

#include "../Data/World.h"

namespace Nork::Physics
{
	class Collision
	{
	public:
		Collision(World& world, uint32_t obj1Idx, uint32_t obj2Idx, float coefficient = 0.1f)
			: world(&world), obj1Idx(obj1Idx), obj2Idx(obj2Idx), coefficient(coefficient)
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
		const Shape& Shape1() const { return GetWorld().shapes[obj1Idx]; }
		const Shape& Shape2() const { return GetWorld().shapes[obj2Idx]; }
		const KinematicData& Kinem1() const { return GetWorld().kinems[obj1Idx]; }
		const KinematicData& Kinem2() const { return GetWorld().kinems[obj2Idx]; }
	private:
		void GenContactPoints();
	public:
		float coefficient;

		uint32_t obj1Idx, obj2Idx;
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

