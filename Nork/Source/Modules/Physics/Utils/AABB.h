#pragma once

#include "../Data/World.h"

namespace Nork::Physics
{
	struct AABB
	{
		AABB(std::span<glm::vec3> verts);
		AABB() = default;
		glm::vec3 min;
		glm::vec3 max;
	};

	class AABBTest
	{
	public:
		AABBTest(Shape& shape1, Shape& shape2)
			: aabb1(AABB(shape1.verts)), aabb2(AABB(shape1.verts)) {}
		static std::vector<std::pair<uint32_t, uint32_t>> GetResult(World&);
		static float GetDelta();
		static uint32_t Intersecting(AABB& aabb1, AABB& aabb2);
	private:
		AABB aabb1;
		AABB aabb2;
	};
}

