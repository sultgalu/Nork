#pragma once

#include "../Data/World.h"

namespace Nork::Physics
{
	struct AABB
	{
		const glm::vec3 min;
		const glm::vec3 max;
	};

	class AABBTest
	{
	public:
		AABBTest(Shape& shape1, Shape& shape2)
			: aabb1(CalcAABB(shape1)), aabb2(CalcAABB(shape2)) { }
		bool GetResult();
	private:
		static AABB CalcAABB(Shape& shape);
	private:
		AABB aabb1;
		AABB aabb2;
	};
}

