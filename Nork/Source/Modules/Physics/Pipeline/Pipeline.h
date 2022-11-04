#pragma once

#include "../World.h"
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

		World world;
	};
}