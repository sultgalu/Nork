#pragma once

#include "Modules/Physics/Data/World.h"
#include "Modules/Physics/Utils.h"

namespace Nork
{
	template<typename Vertex>
	requires requires(Vertex v)
	{
		{ v.pos } -> std::convertible_to<glm::vec3>;
	}
	struct Polygon
	{
		Polygon(float size = 1)
		{
			Add(Vertex(glm::vec3(-size, -size, -size)));
			Add(Vertex(glm::vec3(size, -size, -size)));
			Add(Vertex(glm::vec3(size, size, -size)));
			Add(Vertex(glm::vec3(-size, size, -size)));
			Add(Vertex(glm::vec3(-size, -size, size)));
			Add(Vertex(glm::vec3(size, -size, size)));
			Add(Vertex(glm::vec3(size, size, size)));
			Add(Vertex(glm::vec3(-size, size, size)));

			// Front
			Connect(0, 1);
			Connect(1, 2);
			Connect(2, 3);
			Connect(3, 0);
			Connect(0, 2);
			// Right
			Connect(1, 5);
			Connect(5, 6);
			Connect(6, 2);
			Connect(1, 6);
			// Left
			Connect(0, 4);
			Connect(4, 7);
			Connect(7, 3);
			Connect(0, 7);
			// Top
			Connect(7, 6);
			Connect(3, 6);
			// Bottom
			Connect(4, 5);
			Connect(0, 5);
			// Back
			Connect(4, 6);
		}

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
		inline void Scale(glm::vec3 scale)
		{
			for (size_t i = 0; i < vertices.size(); i++)
			{
				vertices[i].pos *= scale;
			}
		}

		void AddToWorld(Physics::World& world, glm::mat4 model)
		{
			std::vector<Physics::Face> faces;
			std::vector<glm::vec3> fNorms;
			std::vector<glm::vec3> verts;

			verts.reserve(vertices.size());
			for (size_t i = 0; i < vertices.size(); i++)
			{
				verts.push_back(model * glm::vec4(vertices[i].pos, 1));
			}

			auto center = Physics::Center(verts);
			for (size_t i = 0; i < triangleIndices.size(); i++)
			{
				auto normal = glm::normalize(
					glm::cross(
						verts[triangleIndices[i][0]] - verts[triangleIndices[i][1]],
						verts[triangleIndices[i][0]] - verts[triangleIndices[i][2]]
					));

				float dFromC = Physics::SignedDistance(normal, verts[triangleIndices[i][0]], center);
				if (dFromC > 0)
					normal *= -1; // correct normal to face against the center of the poly.
				fNorms.push_back(normal);

				faces.push_back({ Physics::Face{
					static_cast<Physics::index_t>(triangleIndices[i][0]),
					static_cast<Physics::index_t>(triangleIndices[i][1]),
					static_cast<Physics::index_t>(triangleIndices[i][2]),
				} });
			}
			world.AddShape(verts, edgeIndices, faces, fNorms, center);
		}

		Physics::World AsWorld()
		{
			Physics::World result;
			
			std::vector<Physics::Face> faces;
			std::vector<glm::vec3> fNorms;
			std::vector<glm::vec3> verts;

			verts.reserve(vertices.size());
			for (size_t i = 0; i < vertices.size(); i++)
			{
				verts.push_back(vertices[i].pos);
			}

			for (size_t i = 0; i < triangleIndices.size(); i++)
			{
				auto normal = glm::normalize(
					glm::cross(
						vertices[triangleIndices[i][0]].pos - vertices[triangleIndices[i][1]].pos,
						vertices[triangleIndices[i][0]].pos - vertices[triangleIndices[i][2]].pos
					));

				auto center = Physics::Center(verts);
				float dFromC = Physics::SignedDistance(normal, vertices[triangleIndices[i][0]].pos, center);
				if (dFromC > 0)
					normal *= -1; // correct normal to face against the center of the poly.
				fNorms.push_back(normal);

				faces.push_back({ Physics::Face{
					static_cast<Physics::index_t>(triangleIndices[i][0]),
					static_cast<Physics::index_t>(triangleIndices[i][1]),
					static_cast<Physics::index_t>(triangleIndices[i][2]),
				} });
			}
			result.AddShape(verts, edgeIndices, faces, fNorms);
			return result;
		}

		static Polygon<Vertex> GetCube(glm::vec3 pos = glm::vec3(0))
		{
			Polygon<Vertex> meshes;
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
}