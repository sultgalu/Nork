#pragma once

#include "../Data/Object.h"

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
		AABBTest(Collider& collider1, Collider& collider2)
			: aabb1(AABB(collider1.verts)), aabb2(AABB(collider1.verts)) {}
		static std::vector<std::pair<uint32_t, uint32_t>> GetResult(const std::span<Object> objs);
		static float GetDelta();
		static uint32_t Intersecting(AABB& aabb1, AABB& aabb2);
	private:
		AABB aabb1;
		AABB aabb2;
	};
}

