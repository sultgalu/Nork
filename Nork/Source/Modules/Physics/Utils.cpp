#include "Utils.h"

namespace Nork::Physics
{
	using index_t = uint32_t;

	glm::vec3& Farthest(const std::span<const glm::vec3> verts, const glm::vec3&& dir)
	{
		const glm::vec3* farthest = &verts[0];
		float largestDot = glm::dot(dir, *farthest);

		for (uint32_t i = 1; i < verts.size(); i++)
		{
			float dot = glm::dot(dir, verts[i]);
			if (dot > largestDot)
			{
				largestDot = dot;
				farthest = &verts[i];
			}
		}

		return const_cast<glm::vec3&>(*farthest);
	}

	float SignedDistance(const glm::vec3& dir, const glm::vec3& from, const glm::vec3& to)
	{
		glm::vec3 vec = to - from;
		return glm::dot(glm::normalize(dir), vec);
	}
	
	glm::vec3 Center(const std::span<const glm::vec3> points)
	{
		glm::vec3 sum = glm::vec3(0);
		for (size_t i = 0; i < points.size(); i++)
			sum += points[i];
		sum /= points.size();
		return sum;
	}
}