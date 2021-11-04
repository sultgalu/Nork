#pragma once

#include "../Utils.h"

namespace Nork::Physics
{
	using index_t = uint32_t;
	using Edge = std::array<index_t, 2>;
	using Face = std::vector<index_t>;

	struct IndexedVerts
	{
		IndexedVerts(std::span<index_t> idxs, std::span<glm::vec3> points)
			:idxs(idxs), points(points)
		{
		}
		std::span<index_t> idxs;
		std::span<glm::vec3> points;

		inline glm::vec3& operator[](index_t idx)
		{
			return points[idxs[idx]];
		}
		inline index_t GetPointIdx(index_t idx)
		{
			return idxs[idx];
		}
		inline uint32_t size()
		{
			return idxs.size();
		}
	};

	struct Edges
	{
		Edges(std::span<Edge> edges, std::span<glm::vec3> points)
			:edges(edges), points(points)
		{
		}
		std::span<Edge> edges;
		std::span<glm::vec3> points;

		inline std::pair<glm::vec3&, glm::vec3&> operator[](uint32_t idx)
		{
			return std::pair<glm::vec3&, glm::vec3&>(points[edges[idx][0]], points[edges[idx][1]]);
		}
		inline uint32_t EdgeCount()
		{
			return edges.size();
		}
	};

	struct Shape
	{
		std::span<glm::vec3> verts;
		std::span<Edge> edges;
		std::span<Face> faces;
		std::span<glm::vec3> fNorm;
		glm::vec3 center;

		std::vector<Edge> Edges(Face& face) const
		{
			std::vector<Edge> result;

			for (size_t i = 0; i < edges.size(); i++)
			{
				uint32_t count = 0;
				for (size_t j = 0; j < face.size(); j++)
				{
					if ((face[j] == edges[i][0] || face[j] == edges[i][1]) && ++count == 2)
					{
						result.push_back(std::ref(edges[i]));
					}
				}
			}

			return result;
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
		std::pair<const Face&, const Face&> Faces(Edge& edge) const
		{
			std::array<const Face*, 2> result;

			for (size_t i = 0; i < faces.size(); i++)
			{
				uint32_t count = 0;
				for (size_t j = 0; j < faces[i].size(); j++)
				{
					if ((faces[i][j] == edge[0] || faces[i][j] == edge[1]) && ++count == 2)
					{
						result[count - 1] = (&faces[i]);
					}
				}
			}

			return std::pair<const Face&, const Face&> {*result[0], *result[1]};
		}
		std::vector<std::reference_wrapper<const glm::vec3>> VerticesVector(Face& face) const
		{
			std::vector<std::reference_wrapper<const glm::vec3>> res;
			for (size_t i = 0; i < face.size(); i++)
			{
				res.push_back(std::ref(verts[face[i]]));
			}
			return res;
		}
		inline IndexedVerts Vertices(Face& face)
		{
			return IndexedVerts(face, verts);
		}
		inline std::pair<glm::vec3&, glm::vec3&> Vertices(Edge& edge) const
		{
			return std::pair<glm::vec3&, glm::vec3&> { verts[edge[0]], verts[edge[1]] };
		}
		inline const glm::vec3& SomePointOnFace(Face& face) const
		{
			return verts[face[0]];
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
		inline const glm::vec3& FaceCenter(Face& face) const
		{
			glm::vec3 res = glm::zero<glm::vec3>();
			for (size_t i = 0; i < face.size(); i++)
			{
				res += verts[face[i]];
			}
			return res /= face.size();
		}
		inline glm::vec3& FaceNormal(Face& face) const
		{
			for (size_t i = 0; i < faces.size(); i++)
			{
				if (faces[i] == face)
					return fNorm[i];
			}
			std::abort();
		}
	};

	struct KinematicData
	{
		glm::vec3 position;
		glm::quat quaternion;
		glm::vec3 velocity;
		glm::vec3 aVelUp;
		float aVelSpeed;
		float mass;
		bool isStatic = false;
		glm::vec3 forces;
	};

	class World
	{
	public:
		void ClearShapeData()
		{
			size_t vs = verts.size();
			size_t es = edges.size();
			size_t fs = faces.size();
			size_t fns = fNorm.size();

			verts.clear();
			edges.clear();
			faces.clear();
			fNorm.clear();
			shapes.clear();

			verts.reserve(vs);
			edges.reserve(es);
			faces.reserve(fs);
			fNorm.reserve(fns);
		}
		std::vector<glm::vec3> verts;
		std::vector<Edge> edges;
		std::vector<Face> faces;
		std::vector<glm::vec3> fNorm;

		std::vector<Shape> shapes;
		std::vector<KinematicData> kinems;

		Shape& AddShape(std::vector<glm::vec3>& verts, std::vector<Edge>& edges, std::vector<Face>& faces, std::vector<glm::vec3>& fNorm, glm::vec3& center);
		void Remove(Shape& shape);
	};

	
}