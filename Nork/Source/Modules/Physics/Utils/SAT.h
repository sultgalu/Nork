#pragma once

#include "../Data/World.h"

namespace Nork::Physics
{
	class SAT
	{
	public:
		SAT(const Collider& shape1, const Collider& shape2);
		CollisionResult GetResult();
	private:
		bool FacePhase(const Collider& useFaces, const Collider& useVerts, CollisionType resType);
		bool EdgePhase(const Collider& shape1, const Collider& shape2);
	private:
		 CollisionResult state = CollisionResult{ .depth = -std::numeric_limits<float>::max() };
	};

}
