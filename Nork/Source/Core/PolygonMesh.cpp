#include "PolygonMesh.h"

namespace Nork
{
	Polygon::Polygon(float size)
	{
		Add(glm::vec3(-size, -size, -size));
		Add(glm::vec3(size, -size, -size));
		Add(glm::vec3(size, size, -size));
		Add(glm::vec3(-size, size, -size));
		Add(glm::vec3(-size, -size, size));
		Add(glm::vec3(size, -size, size));
		Add(glm::vec3(size, size, size));
		Add(glm::vec3(-size, size, size));

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
	uint32_t Polygon::Add(glm::vec3&& v)
	{
		vertices.push_back(v);
		neighbours.push_back({});
		return vertices.size() - 1;
	}
	void Polygon::Remove(std::span<uint32_t> multiple)
	{
		std::sort(multiple.rbegin(), multiple.rend());
		for (size_t i = 0; i < multiple.size(); i++)
		{
			Remove(multiple[i]);
		}
	}
	void Polygon::Remove(uint32_t idx)
	{
		for (auto nIdx : neighbours[idx])
		{
			neighbours[nIdx].erase(idx);
		}

		std::vector<size_t> triErase;
		std::vector<size_t> edgeErase;
		for (size_t i = 0; i < tris.size(); i++)
		{
			if (tris[i][0] == idx || tris[i][1] == idx || tris[i][2] == idx)
				triErase.push_back(i);
		}
		for (size_t i = 0; i < edges.size(); i++)
		{
			if (edges[i][0] == idx || edges[i][1] == idx)
				edgeErase.push_back(i);
		}
		for (int i = triErase.size() - 1; i >= 0; --i)
			tris.erase(tris.begin() + triErase[i]);
		for (int i = edgeErase.size() - 1; i >= 0; --i)
			edges.erase(edges.begin() + edgeErase[i]);

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
		for (size_t i = 0; i < tris.size(); i++)
		{
			if (tris[i][0] > idx) tris[i][0]--;
			if (tris[i][1] > idx) tris[i][1]--;
			if (tris[i][2] > idx) tris[i][2]--;
		}
		for (size_t i = 0; i < edges.size(); i++)
		{
			if (edges[i][0] > idx) edges[i][0]--;
			if (edges[i][1] > idx) edges[i][1]--;
		}
	}
	void Polygon::Disconnect(uint32_t first, uint32_t second)
	{
		if (!neighbours[first].contains(second))
		{
			Logger::Error(first, " and ", second, " are not neighbours.");
			return;
		}

		neighbours[first].erase(second);
		neighbours[second].erase(first);

		std::vector<size_t> triErase;
		for (size_t i = 0; i < tris.size(); i++)
		{
			int count = 0;
			for (size_t j = 0; j < tris[i].size(); j++)
			{
				if (tris[i][j] == first || tris[i][j] == second)
					count++;
			}
			if (count == 2)
				triErase.push_back(i);
		}

		std::vector<size_t> edgeErase;
		for (size_t i = 0; i < edges.size(); i++)
		{
			if ((edges[i][0] == first && edges[i][1] == second) ||
				(edges[i][0] == second && edges[i][1] == first))
				edgeErase.push_back(i);
		}
		for (int i = triErase.size() - 1; i >= 0; --i)
			tris.erase(tris.begin() + triErase[i]);
		for (int i = edgeErase.size() - 1; i >= 0; --i)
			edges.erase(edges.begin() + edgeErase[i]);
	}
	void Polygon::Connect(uint32_t first, uint32_t second)
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
				tris.push_back({ first, second, third });
			}
		}

		n1.insert(second);
		n2.insert(first);
		edges.push_back({ second, first });
	}
	void Polygon::Scale(glm::vec3 scale)
	{
		for (size_t i = 0; i < vertices.size(); i++)
		{
			vertices[i] *= scale;
		}
	}

	std::vector<std::pair<uint32_t, uint32_t>> Polygon::GetEdges(const std::span<std::vector<uint32_t>>& faces)
	{
		std::vector<std::pair<uint32_t, uint32_t>> edges;
		auto tryAdd = [&](std::pair<uint32_t, uint32_t> edge)
		{
			if (edge.first > edge.second)
			{
				std::swap(edge.first, edge.second);
			}
			for (size_t i = 0; i < edges.size(); i++)
			{
				if (edges[i].first == edge.first && edges[i].second == edge.second)
					return;
			}
			edges.push_back(edge);
		};

		for (size_t i = 0; i < faces.size(); i++)
		{
			uint32_t start = faces[i].size() - 1;
			uint32_t end = 0;
			while (end < faces[i].size())
			{
				tryAdd(std::pair(faces[i][start], faces[i][end]));
				start = end++;
			}
		}

		return edges;
	}

	std::vector<uint32_t> Polygon::SortFaceIdxs(const std::vector<uint32_t>& face)
	{
		std::vector<uint32_t> sortedFace;
		uint32_t start = face[0];
		uint32_t prev = start;
		uint32_t current = start;

		do
		{
			for (auto n : neighbours[current])
			{
				for (size_t j = 0; j < face.size(); j++)
				{
					if (n == face[j] && face[j] != prev)
					{
						sortedFace.push_back(face[j]);
						prev = current;
						current = face[j];
						goto Break;
					}
				}
			}
		Break:;
		} while (current != start);

		return sortedFace;
	}

	std::vector<std::vector<uint32_t>> Polygon::GetFaces()
	{
		auto pointInsidePoly = glm::vec3(0);
		for (size_t i = 0; i < vertices.size(); i++)
			pointInsidePoly += vertices[i];
		pointInsidePoly /= vertices.size();

		std::vector<std::vector<uint32_t>> faces;
		std::vector<glm::vec3> normals;
		auto idxForNormal = [&](glm::vec3& norm)
		{
			for (int i = 0; i < normals.size(); i++)
			{
				if (normals[i] == norm)
					return i;
			}
			return -1;
		};

		for (size_t i = 0; i < tris.size(); i++)
		{
			glm::vec3 normal = glm::normalize(glm::cross(
				vertices[tris[i][0]] - vertices[tris[i][1]],
				vertices[tris[i][0]] - vertices[tris[i][2]]));
			if (glm::dot(vertices[tris[i][0]] - pointInsidePoly, normal) < 0)
				normal *= -1;

			int normalIdx = idxForNormal(normal);
			if (normalIdx > -1)
			{
				auto& face = faces[normalIdx];
				auto containsVert = [&](uint32_t j)
				{
					for (size_t k = 0; k < face.size(); k++)
					{
						if (face[k] == tris[i][j])
							return true;
					}
					return false;
				};
				for (size_t j = 0; j < tris[i].size(); j++)
				{
					if (!containsVert(j))
					{
						face.push_back(tris[i][j]);
					}
				}
			}
			else
			{
				normals.push_back(normal);
				faces.push_back({ tris[i][0], tris[i][1], tris[i][2] });
			}
		}
		for (size_t i = 0; i < faces.size(); i++)
		{
			faces[i] = SortFaceIdxs(faces[i]);
		}
		return faces;
	}

	Physics::Collider Polygon::AsCollider(const glm::vec3& scale)
	{
		Physics::Collider res;

		res.verts.reserve(vertices.size());
		for (size_t i = 0; i < vertices.size(); i++)
		{
			res.verts.push_back(glm::vec4(vertices[i] * scale, 1));
		}

		auto center = glm::vec3(Physics::Center(res.verts));
		auto faces = GetFaces();
		res.faceVerts = faces;
		res.faces.reserve(faces.size());
		for (size_t i = 0; i < faces.size(); i++)
		{
			auto normal = glm::normalize(glm::cross(
				vertices[faces[i][0]] - vertices[faces[i][1]],
				vertices[faces[i][0]] - vertices[faces[i][2]]
			));
			if (glm::dot(normal, vertices[faces[i][0]] - center) < 0)
				normal *= -1; // correct normal to face against the center of the poly.
			res.faces.push_back(Physics::Face{
				.norm = normal,
				.vertIdx = faces[i][0]
				});
		}

		auto actualEdges = GetEdges(faces);
		res.edges.resize(actualEdges.size());
		std::memcpy(res.edges.data(), actualEdges.data(), actualEdges.size() * sizeof(Physics::Edge));

		return res;
	}

	Polygon Polygon::GetCube(glm::vec3 pos)
	{
		Polygon meshes;
		meshes.Add(pos + glm::vec3(-1, -1, -1));
		meshes.Add(pos + glm::vec3(1, -1, -1));
		meshes.Add(pos + glm::vec3(1, 1, -1));
		meshes.Add(pos + glm::vec3(-1, 1, -1));

		meshes.Add(pos + glm::vec3(-1, -1, 1));
		meshes.Add(pos + glm::vec3(1, -1, 1));
		meshes.Add(pos + glm::vec3(1, 1, 1));
		meshes.Add(pos + glm::vec3(-1, 1, 1));

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
}