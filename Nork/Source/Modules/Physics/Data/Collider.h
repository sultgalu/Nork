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

	struct Face
	{
		std::vector<uint32_t> idxs;
		glm::vec3 normal;
	};

	struct Collider
	{
		std::vector<glm::vec3> points;
		std::vector<Face> faces;
		std::vector<std::array<uint32_t, 2>> edges;
		glm::vec3 center;
	};

	static uint32_t SupportPoint(glm::vec3 dir, std::span<glm::vec3> points)
	{
		uint32_t result = 0;
		float largest = -std::numeric_limits<float>::max();
		for (uint32_t i = 0; i < points.size(); i++)
		{
			float dot = glm::dot(dir, points[i]);
			if (dot > largest)
			{
				largest = dot;
				result = i;
			}
		}

		return result;
	}

	static float SignedDistance(glm::vec3& norm, glm::vec3& facePoint, glm::vec3& point)
	{
		auto vec = point - facePoint;
		return glm::dot(norm, vec);
	}
	static std::vector<std::array<Face, 2>> TriangleIntersections(Collider& c1, Collider& c2)
	{
		std::vector<std::array<Face, 2>> res;
		for (auto& face1 : c1.faces)
		{
			for (auto& face2 : c2.faces)
			{
				float sd0 = SignedDistance(face1.normal, c1.points[face1.idxs[0]], c2.points[face2.idxs[0]]);
				float sd1 = SignedDistance(face1.normal, c1.points[face1.idxs[0]], c2.points[face2.idxs[1]]);
				float sd2 = SignedDistance(face1.normal, c1.points[face1.idxs[0]], c2.points[face2.idxs[2]]);

				if ((sd0 > 0 && sd1 > 0 && sd2 > 0) || (sd0 < 0 && sd1 < 0 && sd2 < 0))
					continue; // no intersection

				res.push_back({ face1, face2 });
			}
		}
		return res;
	}

	struct FaceQuery
	{
		int faceIdx = -1;
		float distance = std::numeric_limits<float>::max();
		glm::vec3 normal;
	};

	struct EdgeQuery
	{
		float distance = std::numeric_limits<float>::max();
		int edgeIdx1 = -1;
		std::array<uint32_t, 2> edge1;
		std::array<uint32_t, 2> edge2;
		glm::vec3 normal;
	};

	static EdgeQuery GetEQ(Collider& c1, Collider& c2)
	{
		EdgeQuery result;
		for (auto& edgePs1 : c1.edges)
		{
			for (auto& edgePs2 : c2.edges)
			{
				auto edge1 = c1.points[edgePs1[0]] - c1.points[edgePs1[1]];
				auto edge2 = c2.points[edgePs2[0]] - c2.points[edgePs2[1]];

				auto norm = normalize(glm::cross(edge1, edge2));
				float dFromC = Physics::SignedDistance(norm, c1.points[edgePs1[0]], c1.center);
				if (dFromC > 0)
					norm = -norm;

				auto support = SupportPoint(-norm, c2.points);
				auto sd = SignedDistance(norm, c1.points[edgePs1[0]], c2.points[support]);
				if (sd > 0)
					return EdgeQuery{ .edge1 = edgePs1, .edge2 = edgePs2, .normal = norm };
				if (sd < result.distance)
				{
					result.distance = sd;
					result.edge1 = edgePs1;
					result.edge2 = edgePs2;
					result.normal = norm;
					result.edgeIdx1 = 0;
				}
			}
		}
		if (result.distance > 0)
			result.edgeIdx1 = -1;

		return result;
	}

	static FaceQuery GetFQ(Collider& c1, Collider& c2)
	{
		FaceQuery res;
		for (size_t i = 0; i < c1.faces.size(); i++)
		{
			auto face1 = c1.faces[i];

			auto idx = SupportPoint(-face1.normal, c2.points);
			float sDist = SignedDistance(face1.normal, c1.points[face1.idxs[0]], c2.points[idx]);
			if (sDist > 0)
			{
				return FaceQuery();
			}
			if (sDist < res.distance)
			{
				res.distance = sDist;
				res.faceIdx = i;
				res.normal = face1.normal;
			}
		}

		return res;
	}

	static std::vector<float> GetSDs(Collider& c1, Collider& c2)
	{
		std::vector<float> res;
		for (size_t i = 0; i < c1.faces.size(); i++)
		{
			auto face1 = c1.faces[i];

			auto idx = SupportPoint(-face1.normal, c2.points);
			float sDist = SignedDistance(face1.normal, c1.points[face1.idxs[0]], c2.points[idx]);
			res.push_back(sDist);
		}

		return res;
	}

	static void NarrowPhase(Collider& c1, Collider& c2)
	{
		FaceQuery faceQ = GetFQ(c1, c2);
	}
}