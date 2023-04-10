#include "AABB.h"

namespace Nork::Physics
{
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