#pragma once

namespace Nork::Physics
{
	using index_t = uint32_t;
	glm::vec3 EdgeNormal(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& pInner, const glm::vec3& planeNormal, bool normalize = true);
	glm::vec3 EdgeNormal(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& pInner, bool normalize = true);
	int TryExpandConvex2DSorted(std::vector<glm::vec3>& points, const glm::vec3& point, const glm::vec3& planeNormal);
	std::vector<glm::vec3> SortPoints2D(const std::span<const glm::vec3> points);
	float TriangleArea(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
	glm::vec3 Center2D(const std::span<const glm::vec3> points);
	glm::vec3 Center2DSorted(const std::span<const glm::vec3> points);
	glm::vec3 Center(const std::span<const glm::vec3> points);
	glm::vec4 Center(const std::span<const glm::vec4> points);
	index_t FarthestIdx(const std::span<const glm::vec3> verts, const glm::vec3&& dir);
	glm::vec3& Farthest(const std::span<const glm::vec3> verts, const glm::vec3&& dir);
	float Sign(const glm::vec3& dir, const glm::vec3& from, const glm::vec3& to);
	float SignedDistance(const glm::vec3& dir, const glm::vec3& from, const glm::vec3& to);
	float SignedDistanceNormalized(const glm::vec3& dir, const glm::vec3& from, const glm::vec3& to);

	inline glm::vec3 EdgeNormalTowards(glm::vec3 target, glm::vec3 towards)
	{
		return glm::cross(glm::cross(target, towards), target);
	}
	inline glm::vec3 EdgeNormalOutwards(glm::vec3 target, glm::vec3 from)
	{
		return glm::cross(glm::cross(from, target), target);
	}
}