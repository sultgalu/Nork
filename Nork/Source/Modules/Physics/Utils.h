#pragma once

namespace Nork::Physics
{
	glm::vec3 Center(const std::span<const glm::vec3> points);
	glm::vec3& Farthest(const std::span<const glm::vec3> verts, const glm::vec3&& dir);
	float SignedDistance(const glm::vec3& dir, const glm::vec3& from, const glm::vec3& to);

	inline glm::vec3 EdgeNormalTowards(glm::vec3 target, glm::vec3 towards)
	{
		return glm::cross(glm::cross(target, towards), target);
	}
	inline glm::vec3 EdgeNormalOutwards(glm::vec3 target, glm::vec3 from)
	{
		return glm::cross(glm::cross(from, target), target);
	}
}