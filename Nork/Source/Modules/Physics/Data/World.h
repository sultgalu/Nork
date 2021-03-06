#pragma once

#include "Common.h"
#include "../Utils.h"

namespace Nork::Physics
{
	struct Shape
	{
		std::span<glm::vec3> colliderVerts;
		std::span<Face> colliderFaces;
		std::span<glm::vec3> verts;
		std::span<Edge> edges;
		std::span<Face> faces;
		std::span<std::vector<index_t>> faceVerts;
		glm::vec3 center;
		glm::vec3 colliderCenter;

		std::vector<uint32_t> SideFacesOfVert(uint32_t vertIdx) const
		{
			std::vector<uint32_t> res;
			for (size_t i = 0; i < faceVerts.size(); i++)
			{
				for (size_t j = 0; j < faceVerts[i].size(); j++)
				{
					if (faceVerts[i][j] == vertIdx)
					{
						res.push_back(i);
						break;
					}
				}
			}
			return res;
		}
		std::vector<uint32_t> EdgesOnFace(uint32_t faceIdx) const
		{
			std::vector<uint32_t> res;
			for (size_t i = 0; i < edges.size(); i++)
			{
				int count = 0;
				for (size_t j = 0; j < faceVerts[faceIdx].size(); j++)
				{
					if (faceVerts[faceIdx][j] == edges[i].first ||
						faceVerts[faceIdx][j] == edges[i].second)
					{
						if (++count == 2)
						{
							res.push_back(i);
							break;
						}
					}
				}
			}
			return res;
		}
		std::vector<Edge> Edges(glm::vec3& vert) const
		{
			std::vector<Edge> result;

			for (size_t i = 0; i < edges.size(); i++)
			{
				if (verts[edges[i][0]] == vert || verts[edges[i][1]] == vert)
				{
					result.push_back(std::ref(edges[i]));
				}
			}

			return result;
		}
		inline std::pair<glm::vec3&, glm::vec3&> Vertices(Edge& edge) const
		{
			return std::pair<glm::vec3&, glm::vec3&> { verts[edge[0]], verts[edge[1]] };
		}
		inline const glm::vec3& VertFromFace(Face& face) const
		{
			return verts[face.vertIdx];
		}
		inline const glm::vec3& VertFromFace(uint32_t idx) const
		{
			return verts[faces[idx].vertIdx];
		}
		inline const glm::vec3& FirstVertFromEdge(Edge& edge) const
		{
			return verts[edge[0]];
		}
		inline const glm::vec3& SecondVertFromEdge(Edge& edge) const
		{
			return verts[edge[1]];
		}
		inline glm::vec3 EdgeDirection(Edge& edge) const
		{
			return verts[edge[0]] - verts[edge[1]];
		}
		inline glm::vec3 EdgeMiddle(Edge& edge) const
		{
			return (verts[edge[0]] + verts[edge[1]]) / 2.0f;
		}
	};

	class World
	{
	public:
		std::vector<glm::vec3> colliderVerts;
		std::vector<Face> colliderFaces;
		std::vector<glm::vec3> verts;
		std::vector<Edge> edges;
		std::vector<Face> faces;
		std::vector<std::vector<index_t>> faceVerts;

		std::vector<KinematicData> kinems;
		std::vector<Shape> shapes;

		void ClearColliderData();
		void AddCollider(const Collider& collider);
		void UpdateTransforms(std::span<glm::vec3> translate, std::span<glm::quat> quaternions);
		void Remove(Shape& shape);
	};
}