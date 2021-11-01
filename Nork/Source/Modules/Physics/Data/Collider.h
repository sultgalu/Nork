#pragma once

namespace Nork::Physics
{
	using Edge = std::array<uint32_t, 2>;
	struct Face2
	{
		std::vector<uint32_t> idxs;
		glm::vec3 normal;
	};
	struct BoxCollider
	{
		float x, y, z;
	};

	struct Collider
	{
		std::vector<glm::vec3> points;
		std::vector<Face2> faces;
		std::vector<Edge> edges;
		glm::vec3 center;
		std::vector<Edge> GetEdgesForFace(Face2& face)
		{
			std::vector<Edge> result;
			for (size_t i = 0; i < edges.size(); i++)
			{
				int count = 0;
				for (size_t j = 0; j < face.idxs.size(); j++)
				{
					if ((edges[i][0] == face.idxs[j] || edges[i][1] == face.idxs[j])
						&& ++count == 2)
					{
						result.push_back(edges[i]);
						break;
					}
				}
				return result;
			}
		}
	};
}