#pragma once

#include "../Data/Object.h"
#include "../Data/CollisionResult.h"

namespace Nork::Physics
{
	class SAT
	{
	public:
		SAT(const Collider& collider1, const Collider& collider2);
		CollisionResult GetResult();
	private:
		bool FacePhase(const Collider& useFaces, const Collider& useVerts, CollisionType resType);
		bool EdgePhase(const Collider& collider1, const Collider& collider2);
	private:
		 CollisionResult state = CollisionResult{ .depth = -std::numeric_limits<float>::max() };
	};

}
