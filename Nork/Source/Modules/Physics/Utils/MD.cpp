#include "pch.h"
#include "MD.h"

namespace Nork::Physics
{
	std::vector<glm::vec3> MD::MinkowskiDifference(std::span<glm::vec3> verts1, std::span<glm::vec3> verts2)
	{
		std::vector<glm::vec3> res;

		for (size_t i = 0; i < verts1.size(); i++)
		{
			for (size_t j = 0; j < verts2.size(); j++)
			{
				res.push_back(verts1[i] - verts2[j]);
			}
		}

		return res;
	}
}