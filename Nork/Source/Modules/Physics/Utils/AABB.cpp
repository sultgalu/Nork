#include "pch.h"
#include "AABB.h"

namespace Nork::Physics
{
	static float delta = 0;
	float AABBTest::GetDelta()
	{
		return delta;
	}
	uint32_t AABBTest::Intersecting(AABB& aabb1, AABB& aabb2)
	{
		for (int i = 0; i < 3; i++)
		{
			if (aabb1.min[i] < aabb2.min[i])
			{
				if (aabb1.max[i] > aabb2.min[i])
					continue;
			}
			else
			{
				if (aabb1.min[i] < aabb2.max[i])
					continue;
			}

			return 0;
		}
		return 3;
	}

	std::vector<std::pair<uint32_t, uint32_t>> AABBTest::GetResult(World& world)
	{
		Timer t;
		std::vector<AABB> aabbs;
		aabbs.reserve(world.colliders.size());
		std::vector<std::pair<uint32_t, uint32_t>> results;

		for (size_t i = 0; i < world.colliders.size(); i++)
		{
			aabbs.push_back(AABB(world.colliders[i].verts));
		}
		for (size_t i = 0; i < aabbs.size(); i++)
		{
			for (size_t j = i + 1; j < aabbs.size(); j++)
			{
				if (Intersecting(aabbs[i], aabbs[j]))
					results.push_back(std::pair(i, j));
			}
		}
		delta = t.Elapsed();
		return results;
	}

	AABB::AABB(std::span<glm::vec3> verts)
	{
		min = verts[0];
		max = verts[0];

		for (size_t i = 1; i < verts.size(); i++)
		{
			if (verts[i].x < min.x)
				min.x = verts[i].x;
			if (verts[i].y < min.y)
				min.y = verts[i].y;
			if (verts[i].z < min.z)
				min.z = verts[i].z;

			if (verts[i].x > max.x)
				max.x = verts[i].x;
			if (verts[i].y > max.y)
				max.y = verts[i].y;
			if (verts[i].z > max.z)
				max.z = verts[i].z;
		}
	}
}