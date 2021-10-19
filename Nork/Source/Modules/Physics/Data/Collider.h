#pragma once

namespace Nork::Physics
{
	struct BoxCollider
	{
		float x, y, z;
	};

	struct AABB
	{
		glm::vec3 min, max;
	};

	static inline std::array<glm::vec3, 8> EdgesFromBox(const BoxCollider& bc)
	{
		return std::array<glm::vec3, 8> {
			glm::vec3( bc.x,  bc.y,  bc.z),
			glm::vec3( bc.x, -bc.y,  bc.z),
			glm::vec3(-bc.x, -bc.y,  bc.z),
			glm::vec3(-bc.x,  bc.y,  bc.z),
			glm::vec3( bc.x,  bc.y, -bc.z),
			glm::vec3( bc.x, -bc.y, -bc.z),
			glm::vec3(-bc.x, -bc.y, -bc.z),
			glm::vec3(-bc.x,  bc.y, -bc.z),
		};
	}

	static AABB CalcAABB(const BoxCollider& bc, const glm::vec3& pos, glm::mat4& rot)
	{
		auto edges = EdgesFromBox(bc);

		AABB aabb { .min = glm::vec3(0), .max = glm::vec3(0) };

		for (size_t i = 0; i < edges.size(); i++)
		{
			edges[i] = (rot * glm::vec4(edges[i], 0.0f));

			if (edges[i].x < aabb.min.x)
				aabb.min.x = edges[i].x;
			if (edges[i].y < aabb.min.y)
				aabb.min.y = edges[i].y;
			if (edges[i].z < aabb.min.z)
				aabb.min.z = edges[i].z;

			if (edges[i].x > aabb.max.x)
				aabb.max.x = edges[i].x;
			if (edges[i].y > aabb.max.y)
				aabb.max.y = edges[i].y;
			if (edges[i].z > aabb.max.z)
				aabb.max.z = edges[i].z;
		}

		aabb.min += pos;
		aabb.max += pos;

		return aabb;
	}

	static bool BroadTest(BoxCollider& bc1, BoxCollider& bc2, glm::vec3& pos1, glm::vec3& pos2, glm::mat4&& rot1, glm::mat4&& rot2)
	{
		AABB aabb1 = CalcAABB(bc1, pos1, rot1);
		AABB aabb2 = CalcAABB(bc2, pos2, rot2);

		float* min1 = &aabb1.min.x;
		float* max1 = &aabb1.max.x;
		float* min2 = &aabb2.min.x;
		float* max2 = &aabb2.max.x;

		int count = 0;
		for (size_t i = 0; i < 3; i++)
		{
			if (min1[i] < min2[i])
			{
				if (max1[i] > min2[i])
					count++;
			}
			else
			{
				if (min1[i] < max2[i])
					count++;
			}
		}

		return count == 3;
	}

	struct Collider
	{
		std::vector<float> edges;
	};
}