#pragma once

#include "../Data/World.h"
#include "../Utils/AABB.h"
#include "../Utils/GJK.h"
#include "../Utils/SAT.h"
#include "../Utils/SAP.h"
#include "Collision.h"

namespace Nork::Physics
{
	class Pipeline
	{

	public:
		// std::vector<Collider> colls;
		std::vector<Collider> collidersLocal;
		std::vector<Collision> collisions;

		bool updateVelocities = true, updateRotation = true;
		bool detectCollisions = true, handleCollisions = true;
		bool genContactPoints = true;

		float g = 10;
		float coefficient = 0.1f;

		std::vector<std::pair<index_t, index_t>> broadResults;

		void Update(float delta);

		void VelocityUpdate(KinematicData& kinem, float delta);
		void RotationUpdate(KinematicData& kinem, float delta);

		void SetColliders();
		void SetModels();

		World world;
	};
}