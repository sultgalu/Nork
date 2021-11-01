#include "pch.h"
#include "AABB.h"

namespace Nork::Physics
{
	AABB AABBTest::CalcAABB(Shape& shape)
	{
		glm::vec3 min = shape.verts[0], max = shape.verts[0];

		for (size_t i = 1; i < shape.verts.size(); i++)
		{
			if (shape.verts[i].x < min.x)
				min.x = shape.verts[i].x;
			if (shape.verts[i].y < min.y)
				min.y = shape.verts[i].y;
			if (shape.verts[i].z < min.z)
				min.z = shape.verts[i].z;

			if (shape.verts[i].x > max.x)
				max.x = shape.verts[i].x;
			if (shape.verts[i].y > max.y)
				max.y = shape.verts[i].y;
			if (shape.verts[i].z > max.z)
				max.z = shape.verts[i].z;
		}

		return AABB { .min = min, .max = max };
	}

	bool AABBTest::GetResult()
	{
		int count = 0;
		for (size_t i = 0; i < 3; i++)
		{
			if (aabb1.min[i] < aabb2.min[i])
			{
				if (aabb1.max[i] > aabb2.min[i])
					count++;
			}
			else
			{
				if (aabb1.min[i] < aabb2.max[i])
					count++;
			}
		}

		return count == 3;
	}

}