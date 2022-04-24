#include "Utils.h"

namespace Nork::Physics
{
	index_t FarthestIdx(const std::span<const glm::vec3> verts, const glm::vec3&& dir)
	{
		index_t farthest = 0;
		float largestDot = glm::dot(dir, verts[0]);

		for (uint32_t i = 1; i < verts.size(); i++)
		{
			float dot = glm::dot(dir, verts[i]);
			if (dot > largestDot)
			{
				largestDot = dot;
				farthest = i;
			}
		}

		return farthest;
	}

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

	float Sign(const glm::vec3& dir, const glm::vec3& from, const glm::vec3& to)
	{
		glm::vec3 vec = to - from;
		return glm::dot(dir, vec);
	}
	float SignedDistance(const glm::vec3& dir, const glm::vec3& from, const glm::vec3& to)
	{
		glm::vec3 vec = to - from;
		return glm::dot(glm::normalize(dir), vec);
	}
	float SignedDistanceNormalized(const glm::vec3& dir, const glm::vec3& from, const glm::vec3& to)
	{
		glm::vec3 vec = to - from;
		return glm::dot(dir, vec);
	}
	
	glm::vec3 Center(const std::span<const glm::vec3> points)
	{
		glm::vec3 sum = glm::vec3(0);
		for (size_t i = 0; i < points.size(); i++)
			sum += points[i];
		sum /= points.size();
		return sum;
	}

	glm::vec3 EdgeNormal(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& pInner, const glm::vec3& planeNormal, bool normalize)
	{
		auto edgeNormal = glm::cross(planeNormal, p1 - p2);
		if (Physics::Sign(edgeNormal, p1, pInner) > 0)
		{ // making sure faces outwards
			edgeNormal *= -1;
		}
		if (normalize)
		{
			edgeNormal = glm::normalize(edgeNormal);
		}
		return edgeNormal;
	}
	glm::vec3 EdgeNormal(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& pInner, bool normalize)
	{
		auto planeNormal = glm::cross(p1 - p2, p1 - pInner);
		return EdgeNormal(p1, p2, pInner, planeNormal);
	}
	// loop over the edges, find the first edge, where point is outside, thats the insertion point
	// continue searching, if it finds an other edge like that, it means that point would make the shape concave, return -1
	// without finding an edge like that, it means the point is inside, and it would make the shape concave, return -1
	int TryExpandConvex2DSorted(std::vector<glm::vec3>& points, const glm::vec3& point, const glm::vec3& planeNormal)
	{
		if (points.size() <= 2)
		{
			points.push_back(point);
			return points.size() - 1;
		}

		int insertIdx = -1;
		for (size_t i = 0; i < points.size(); i++)
		{
			auto& p1 = points[i];
			auto& p2 = points[(i + 1) % points.size()];

			auto normal = EdgeNormal(p1, p2, points[(i + 2) % points.size()], planeNormal, false);
			if (Sign(normal, p1, point) > 0)
			{
				for (size_t k = 0; k < points.size(); k++)
				{
					if (points[k] == point)
					{
						Logger::Error("");
					}
				}
				if (insertIdx != -1)
				{ // multiple possible insertion points
					return false;
				}
				insertIdx = (i + 1) % points.size();
			}
		}
		if (insertIdx != -1)
		{
			points.insert(points.begin() + insertIdx, point);
		}
		return insertIdx;
	}
	std::vector<glm::vec3> SortPoints2D(const std::span<const glm::vec3> points)
	{
		if (points.size() <= 2)
		{
			return std::vector<glm::vec3>(points.begin(), points.end());
		}
		auto planeNormal = glm::normalize(glm::cross(points[0] - points[1], points[0] - points[2]));
		auto innerPoint = Center(points);
		std::vector<glm::vec3> sorted;
		std::vector<uint32_t> unsorted(points.size());
		for (size_t i = 0; i < points.size(); i++) // ommit first
			unsorted[i] = i;
		
		auto allOthersAreInside = [&](const glm::vec3& point1, const glm::vec3& point2)
		{
			auto normal = glm::cross(point2 - point1, planeNormal);
			if (Sign(normal, point1, innerPoint) < 0)
			{
				normal *= -1; // make sure it points inwards
			}
			for (auto& inner : points)
			{ // all other points should be on the positive side of the edge normal
				if (inner == point1 || inner == point2)
					continue;
				if (Sign(normal, point1, inner) < 0)
				{
					return false;
				}
			}
			return true;
		};

		auto first = points.back();
		sorted.push_back(first);
		unsorted.pop_back();

		while (unsorted.size() > 0)
		{
			auto size = unsorted.size();
			for (size_t i = 0; i < unsorted.size(); i++)
			{// find the next point from "first"
				auto second = points[unsorted[i]];
				if (allOthersAreInside(first, second))
				{
					sorted.push_back(second);
					first = second;
					unsorted.erase(unsorted.begin() + i);
					break;
				}
			}
			if (size == unsorted.size())
			{
				std::abort(); // algo stuck
			}
		}
		return sorted;
	}
	float TriangleArea(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
	{
		auto e1 = glm::length(p1 - p2);
		auto e2 = glm::length(p1 - p3);
		auto e3 = glm::length(p2 - p3);
		auto s = (e1 + e2 + e3) / 2.0f;
		auto resPow = s * (s - e1) * (s - e2) * (s - e3);
		if (resPow <= 0)
			return 0;
		auto res = glm::sqrt(resPow);
		return res;
	}
	glm::vec3 Center2D(const std::span<const glm::vec3> points)
	{
		auto sortedPoints = SortPoints2D(points);
		return Center2DSorted(sortedPoints);
	}
	glm::vec3 Center2DSorted(const std::span<const glm::vec3> sortedPoints)
	{
		if (sortedPoints.size() <= 3)
		{
			return Center(sortedPoints);
		}
		auto& p1 = sortedPoints[0];
		glm::vec3 centerSum = glm::vec3(0);
		float areaSum = 0;
		for (size_t i = 1; i < sortedPoints.size() - 1; i++)
		{
			auto& p2 = sortedPoints[i];
			auto& p3 = sortedPoints[i + 1];
			auto triCenter = (p1 + p2 + p3) / 3.0f;
			auto triArea = TriangleArea(p1, p2, p3);
			centerSum += triCenter * triArea;
			areaSum += triArea; 
		}
		auto center = centerSum / areaSum;
		return center;
	}
	glm::vec4 Center(const std::span<const glm::vec4> points)
	{
		glm::vec4 sum = glm::vec4(0);
		for (size_t i = 0; i < points.size(); i++)
			sum += points[i];
		sum /= points.size();
		return sum;
	}
}