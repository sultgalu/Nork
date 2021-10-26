#pragma once

#include "Modules/Physics/Data/Collider.h"

namespace Nork
{
	template<typename Vertex>
	/*requires requires(Vertex v)
	{
		{ v.pos } -> std::same_as<glm::vec3>;
	}*/
	struct MeshWorld
	{
		std::vector<Vertex> vertices;
		std::vector<std::unordered_set<uint32_t>> neighbours;

		std::vector<std::array<uint32_t, 3>> triangleIndices;
		std::vector<std::array<uint32_t, 2>> edgeIndices;

		inline uint32_t Add(Vertex v)
		{
			vertices.push_back(v);
			neighbours.push_back({});
			return vertices.size() - 1;
		}
		inline void Remove(std::span<uint32_t> multiple)
		{
			std::sort(multiple.rbegin(), multiple.rend());
			for (size_t i = 0; i < multiple.size(); i++)
			{
				Remove(multiple[i]);
			}
		}
		inline void Remove(uint32_t idx)
		{
			for (auto nIdx : neighbours[idx])
			{
				neighbours[nIdx].erase(idx);
			}

			std::vector<size_t> triErase;
			std::vector<size_t> edgeErase;
			for (size_t i = 0; i < triangleIndices.size(); i++)
			{
				if (triangleIndices[i][0] == idx || triangleIndices[i][1] == idx || triangleIndices[i][2] == idx)
					triErase.push_back(i);
			}
			for (size_t i = 0; i < edgeIndices.size(); i++)
			{
				if (edgeIndices[i][0] == idx || edgeIndices[i][1] == idx)
					edgeErase.push_back(i);
			}
			for (int i = triErase.size() - 1; i >= 0; --i)
				triangleIndices.erase(triangleIndices.begin() + triErase[i]);
			for (int i = edgeErase.size() - 1; i >= 0; --i)
				edgeIndices.erase(edgeIndices.begin() + edgeErase[i]);

			vertices.erase(vertices.begin() + idx);
			neighbours.erase(neighbours.begin() + idx);

			// decreasing every idx reference that was in a higher position
			for (size_t i = 0; i < neighbours.size(); i++)
			{
				std::vector<uint32_t> toDecrement;
				for (auto n : neighbours[i])                    if (n > idx) toDecrement.push_back(n);
				for (size_t j = 0; j < toDecrement.size(); j++)              neighbours[i].erase(toDecrement[j]);
				for (size_t j = 0; j < toDecrement.size(); j++)              neighbours[i].insert(toDecrement[j] - 1);
			}
			for (size_t i = 0; i < triangleIndices.size(); i++)
			{
				if (triangleIndices[i][0] > idx) triangleIndices[i][0]--;
				if (triangleIndices[i][1] > idx) triangleIndices[i][1]--;
				if (triangleIndices[i][2] > idx) triangleIndices[i][2]--;
			}
			for (size_t i = 0; i < edgeIndices.size(); i++)
			{
				if (edgeIndices[i][0] > idx) edgeIndices[i][0]--;
				if (edgeIndices[i][1] > idx) edgeIndices[i][1]--;
			}
		}
		inline void Disconnect(uint32_t first, uint32_t second)
		{
			if (!neighbours[first].contains(second))
			{
				Logger::Error(first, " and ", second, " are not neighbours.");
				return;
			}

			neighbours[first].erase(second);
			neighbours[second].erase(first);

			std::vector<size_t> triErase;
			for (size_t i = 0; i < triangleIndices.size(); i++)
			{
				int count = 0;
				for (size_t j = 0; j < triangleIndices[i].size(); j++)
				{
					if (triangleIndices[i][j] == first || triangleIndices[i][j] == second)
						count++;
				}
				if (count == 2)
					triErase.push_back(i);
			}

			std::vector<size_t> edgeErase;
			for (size_t i = 0; i < edgeIndices.size(); i++)
			{
				if ((edgeIndices[i][0] == first && edgeIndices[i][1] == second) ||
					(edgeIndices[i][0] == second && edgeIndices[i][1] == first))
					edgeErase.push_back(i);
			}
			for (int i = triErase.size() - 1; i >= 0; --i)
				triangleIndices.erase(triangleIndices.begin() + triErase[i]);
			for (int i = edgeErase.size() - 1; i >= 0; --i)
				edgeIndices.erase(edgeIndices.begin() + edgeErase[i]);
		}
		inline void Connect(uint32_t first, uint32_t second)
		{
			auto& n1 = neighbours[first];
			auto& n2 = neighbours[second];
			if (n1.contains(second))
			{
				Logger::Error(first, " and ", second, " are already neighbours.");
				return;
			}

			for (uint32_t third : n1)
			{
				if (n2.contains(third)) // find common neighbours
				{
					triangleIndices.push_back({ first, second, third });
				}
			}

			n1.insert(second);
			n2.insert(first);
			edgeIndices.push_back({ second, first });
		}

		Physics::Collider AsCollider()
		{
			Physics::Collider result;
			glm::vec3 sum = glm::vec3(0);
			for (size_t i = 0; i < vertices.size(); i++)
			{
				sum += vertices[i].pos;
			}
			result.center = sum;
			result.center /= vertices.size();
			for (size_t i = 0; i < triangleIndices.size(); i++)
			{
				auto normal = glm::normalize(
					glm::cross(
						vertices[triangleIndices[i][0]].pos - vertices[triangleIndices[i][1]].pos,
						vertices[triangleIndices[i][0]].pos - vertices[triangleIndices[i][2]].pos
					));

				float dFromC = Physics::SignedDistance(normal, vertices[triangleIndices[i][0]].pos, result.center);
				if (dFromC > 0)
					normal = -normal; // correct normal to face against the center of the poly.

				result.faces.push_back(Physics::Face{ .idxs = std::vector{
					triangleIndices[i][0],
					triangleIndices[i][1],
					triangleIndices[i][2],
				}, .normal = normal });
			}
			result.points.reserve(vertices.size());
			for (size_t i = 0; i < vertices.size(); i++)
			{
				result.points.push_back(vertices[i].pos);
			}
			result.edges = edgeIndices;

			return result;
		}

		static MeshWorld<Vertex> GetCube(glm::vec3 pos = glm::vec3(0))
		{
			MeshWorld<Vertex> meshes;
			meshes.Add(Vertex(pos + glm::vec3( -1, -1, -1)));
			meshes.Add(Vertex(pos + glm::vec3( 1, -1, -1 )));
			meshes.Add(Vertex(pos + glm::vec3( 1, 1, -1 )));
			meshes.Add(Vertex(pos + glm::vec3( -1, 1, -1 )));

			meshes.Add(Vertex(pos + glm::vec3( -1, -1, 1 )));
			meshes.Add(Vertex(pos + glm::vec3( 1, -1, 1 )));
			meshes.Add(Vertex(pos + glm::vec3( 1, 1, 1 )));
			meshes.Add(Vertex(pos + glm::vec3( -1, 1, 1 )));

			// Front
			meshes.Connect(0, 1);
			meshes.Connect(1, 2);
			meshes.Connect(2, 3);
			meshes.Connect(3, 0);
			meshes.Connect(0, 2);
			// Right
			meshes.Connect(1, 5);
			meshes.Connect(5, 6);
			meshes.Connect(6, 2);
			meshes.Connect(1, 6);
			// Left
			meshes.Connect(0, 4);
			meshes.Connect(4, 7);
			meshes.Connect(7, 3);
			meshes.Connect(0, 7);
			// Top
			meshes.Connect(7, 6);
			meshes.Connect(3, 6);
			// Bottom
			meshes.Connect(4, 5);
			meshes.Connect(0, 5);
			// Back
			meshes.Connect(4, 6);
			return meshes;
		}
	};

	template<typename Vertex>
	struct MeshSubWorld
	{
		MeshWorld<Vertex>& world;
		uint32_t begin, end;

		std::vector<std::array<uint32_t, 3>> triangleIndices;
		std::vector<std::array<uint32_t, 2>> edgeIndices;
	};
}