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
		glm::vec3 center;
		glm::vec3 colliderCenter;

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

		std::vector<KinematicData> kinems;
		std::vector<Shape> shapes;

		void ClearColliderData();
		void AddCollider(const Collider& collider);
		void UpdateTransforms(std::span<glm::vec3> translate, std::span<glm::quat> quaternions);
		void Remove(Shape& shape);
	};
}