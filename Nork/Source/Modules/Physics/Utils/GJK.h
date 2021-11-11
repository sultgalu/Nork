#pragma once

#include "../Utils.h"

namespace Nork::Physics
{
	class GJK
	{
	public:
		GJK(std::span<glm::vec3> verts1, std::span<glm::vec3> verts2)
			: verts1(verts1), verts2(verts2) { }

		std::optional<std::pair<float, glm::vec3>> GetResult()
		{
			return GetResult(Center(verts1), Center(verts2));
		}

		std::optional<std::pair<float, glm::vec3>> GetResult(glm::vec3 center1, glm::vec3 center2)
		{
			return Calculate(center1 - center2);
		}

		std::pair<float, glm::vec3> ClosestDepthAndDir(std::vector<glm::vec3> simplex, std::vector<glm::vec3> verts1, std::vector<glm::vec3> verts2);

	private:
		std::optional<std::pair<float, glm::vec3>> Calculate(glm::vec3 initialDirection);
	public:
		std::span<glm::vec3> verts1, verts2;
	};
}