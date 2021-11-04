#pragma once

#include "../Data/World.h"
#include "../CollisionDetection/AABB.h"
#include "../CollisionDetection/Clip.h"
#include "../CollisionDetection/GJK.h"
#include "../CollisionDetection/SAT.h"

namespace Nork::Physics
{
	class System
	{
	public:
		//bool satRes = false, gjkRes = false, clipRes = false, aabbRes = false;
		//bool sat = true, gjk = false, clip = true, aabb = true;
		bool updateVelocities = true, updateRotation = true;
		bool detectCollisions = true, resolveCollisions = true;
		bool resolvePositions = true, resolveMomentum = true, resolveAngularMomentum = false;
		bool genContactPoints = false;
		bool gjk = false, gjkRes = false;
		bool clip = false;
		bool aabb = false;
		float g = 8;
		float coefficient = 0.5f;
		bool applyForces = true;
		bool applyCounterForces = true;

		struct DetectionResult
		{
			glm::vec3 dir;
			float depth;
		};

		std::vector<std::pair<std::pair<uint32_t, uint32_t>, DetectionResult>> detectionResults;
		std::vector<glm::vec3> contactPoints;
		std::vector<glm::vec3> clipContactPoints;

		void Update(World& world, float delta);
		void GenClipContactPoints(World& world);
		void DetectCollisions(World& world);
		void GenContactPoints(World& world);
		void ResolveCollisions(World& world, float delta);
		void ResolveCollisions2(World& world);
		void VelocityUpdate(World& world, float delta);
		void RotationUpdate(World& world, float delta);
	};
}