#include "Components/Collider.h"
#include "Modules/Physics/Utils.h"

namespace Nork::Components {
	/*void Collider::RebuildEdges()
	{
		edges.clear();
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
			uint32_t start = faces[i].points.size() - 1;
			uint32_t end = 0;
			while (end < faces[i].points.size())
			{
				tryAdd(std::pair(faces[i].points[start], faces[i].points[end]));
				start = end++;
			}
		}
	}*/
	std::vector<uint32_t> Collider::TriangleIndices() const
	{
		std::vector<uint32_t> indices;
		for (auto& face : faces)
		{
			for (size_t i = 1; i < face.points.size() - 1; i++)
			{
				indices.push_back(face.points[0]);
				indices.push_back(face.points[i]);
				indices.push_back(face.points[i + 1]);
			}
		}
		return indices;
	}
	uint32_t Collider::TriangleCount() const
	{
		uint32_t count = 0;
		for (auto& face : faces)
		{
			count += std::max(0, (int)face.points.size() - 2);
		}
		return count;
	}
	std::vector<uint32_t> Collider::EdgeIndices() const
	{
		std::vector<uint32_t> result(edges.size() * 2);
		std::memcpy(result.data(), edges.data(), result.size() * sizeof(uint32_t));
		return result;
	}
	void Collider::BuildTriangleFaces()
	{
		auto center = Center();
		faces.clear();
		for (size_t i = 0; i < edges.size(); i++)
		{
			auto& edge1 = edges[i];
			std::vector<uint32_t> neighs1;
			std::vector<uint32_t> neighs2;
			for (size_t j = i + 1; j < edges.size(); j++)
			{
				auto& edge2 = edges[j];
				if (edge1.first == edge2.first || edge1.first == edge2.second)
					neighs1.push_back(j);
				else if (edge1.second == edge2.first || edge1.second == edge2.second)
					neighs2.push_back(j);
			}
			for (size_t j = 0; j < neighs1.size(); j++)
			{
				auto& edge2 = edges[neighs1[j]];
				for (size_t k = 0; k < neighs2.size(); k++)
				{
					auto& edge3 = edges[neighs2[k]];
					if (edge2.first == edge3.second || edge2.first == edge3.first ||
						edge3.first == edge2.second || edge3.second == edge2.second)
					{
						auto face = Face{ .points = { edge1.first, edge1.second } };
						if (edge2.first != edge1.first && edge2.first != edge1.second)
							face.points.push_back(edge2.first);
						else
							face.points.push_back(edge2.second);
						face.normal = FaceNormal(face);
						if (Nork::Physics::SignedDistanceNormalized(face.normal, points[face.points[0]], center) > 0)
						{
							std::swap(face.points[1], face.points[2]);
							face.normal = FaceNormal(face);
						}
						if (!(face.normal.x < 1.0f || face.normal.y < 1.0f || face.normal.z < 1.0f))
						{
							std::abort();
						}
						faces.push_back(face);
					}
				}
			}
		}
	}
	void Collider::CombineFaces()
	{
		auto expandFaceIdx = [&](Face& face, const glm::vec3 point)
		{
			for (size_t i = 0; i < face.points.size(); i++)
			{
				auto& point1 = points[face.points[i]];
				auto& point2 = points[face.points[(i + 1) % face.points.size()]];
				auto edgeNormal = glm::cross(face.normal, point1 - point2);
				if (Nork::Physics::Sign(edgeNormal, point1, points[face.points[(i + 2) % face.points.size()]]) > 0)
				{
					edgeNormal *= -1;
				}
				if (Nork::Physics::Sign(edgeNormal, point1, point) > 0)
				{
					// remove broken edge from edges
					auto idx1 = face.points[i];
					auto idx2 = face.points[(i + 1) % face.points.size()];
					for (size_t k = 0; k < edges.size(); k++)
					{
						auto& edge = edges[k];
						if ((edge.first == idx1 && edge.second == idx2) || (edge.first == idx2 && edge.second == idx1))
						{
							edges.erase(edges.begin() + k);
							break;
						}
					}

					return (i + 1) % face.points.size();
				}
			}
		};

		std::vector<Face> newFaces;
		auto combine = [&](Face& into, Face& from)
		{
			for (size_t i = 0; i < from.points.size(); i++)
			{
				auto& idx = from.points[i];
				
				bool common = false;
				for (auto idx2 : into.points)
				{
					if (idx == idx2)
					{
						common = true;
						break;
					}
				}
				if (!common)
				{
					auto& point = points[idx];
					auto pos = expandFaceIdx(into, point);
					into.points.insert(into.points.begin() + pos, idx);
				}
			}
		};
		auto tryCombine = [&](size_t i)
		{
			auto& face2 = faces[i];
			for (size_t j = 0; j < newFaces.size(); j++)
			{
				auto& face1 = newFaces[j];
				if (face1.normal == face2.normal)
				{
					combine(face1, face2);
					return true;
				}
			}
			return false;
		};
		for (size_t i = 0; i < faces.size(); i++)
		{
			if (!tryCombine(i))
			{
				newFaces.push_back(faces[i]);
			}
		}
		faces = newFaces;
	}
	glm::vec3 Collider::Center()
	{
		glm::vec3 center(0);
		for (auto& point : points)
		{
			center += point;
		}
		center /= (float)points.size();
		return center;
	}
	glm::vec3 Collider::FaceNormal(const Face& face)
	{
		return glm::normalize(glm::cross(
			points[face.points[0]] - points[face.points[1]],
			points[face.points[0]] - points[face.points[2]]));
	}
	Collider Collider::Cube()
	{
		Collider collider;
		constexpr float size = 1;

		collider.points.push_back(glm::vec3(-size, -size, -size));
		collider.points.push_back(glm::vec3(size, -size, -size));
		collider.points.push_back(glm::vec3(size, size, -size));
		collider.points.push_back(glm::vec3(-size, size, -size));
		collider.points.push_back(glm::vec3(-size, -size, size));
		collider.points.push_back(glm::vec3(size, -size, size));
		collider.points.push_back(glm::vec3(size, size, size));
		collider.points.push_back(glm::vec3(-size, size, size));

		// Front
		collider.AddEdge(0, 1);
		collider.AddEdge(1, 2);
		collider.AddEdge(2, 3);
		collider.AddEdge(3, 0);
		collider.AddEdge(0, 2);
		// Right
		collider.AddEdge(1, 5);
		collider.AddEdge(5, 6);
		collider.AddEdge(6, 2);
		collider.AddEdge(1, 6);
		// Left
		collider.AddEdge(0, 4);
		collider.AddEdge(4, 7);
		collider.AddEdge(7, 3);
		collider.AddEdge(0, 7);
		// Top
		collider.AddEdge(7, 6);
		collider.AddEdge(3, 6);
		// Bottom
		collider.AddEdge(4, 5);
		collider.AddEdge(0, 5);
		// Back
		collider.AddEdge(4, 6);

		collider.BuildTriangleFaces();
		collider.CombineFaces();

		return collider;
	}
	uint32_t Collider::AddPoint(const glm::vec3& p)
	{
		points.push_back(p);
		return points.size() - 1;
	}
	uint32_t Collider::AddEdge(uint32_t l, uint32_t r)
	{
		if (l == r)
			std::abort();
		if (l > r)
		{
			std::swap(l, r);
		}
		uint32_t i = 0;
		while (i < edges.size() && edges[i].first <= l && edges[i].second < r)
		{
			i++;
		}
		if (i == edges.size())
		{
			edges.push_back({ l, r });
			OnEdgeAdded(i);
		}
		if (edges[i].first != l || edges[i].second != r)
		{
			edges.insert(edges.begin() + i, { l ,r });
			OnEdgeAdded(i);
		}
		return i;
	}
	// add new faces created by new edge (if any)
	void Collider::OnEdgeAdded(uint32_t addedEdgeIdx)
	{
		auto center = Center();
		auto& edge1 = edges[addedEdgeIdx];

		std::vector<uint32_t> neighs1;
		std::vector<uint32_t> neighs2;
		for (size_t i = 0; i < edges.size(); i++)
		{
			if (i == addedEdgeIdx)
				continue;
			auto& edge2 = edges[i];
			if (edge1.first == edge2.first || edge1.first == edge2.second)
				neighs1.push_back(i);
			else if (edge1.second == edge2.first || edge1.second == edge2.second)
				neighs2.push_back(i);
		}

		// TODO:: if the 2 points are in a common face, split that face
		for (size_t j = 0; j < neighs1.size(); j++)
		{
			auto& edge2 = edges[neighs1[j]];
			for (size_t k = 0; k < neighs2.size(); k++)
			{
				auto& edge3 = edges[neighs2[k]];
				if (edge2.first == edge3.second || edge2.first == edge3.first ||
					edge3.first == edge2.second || edge3.second == edge2.second)
				{
					auto face = Face{ .points = { edge1.first, edge1.second } };
					if (edge2.first != edge1.first && edge2.first != edge1.second)
						face.points.push_back(edge2.first);
					else
						face.points.push_back(edge2.second);
					face.normal = FaceNormal(face);
					if (Nork::Physics::SignedDistanceNormalized(face.normal, points[face.points[0]], center) > 0)
					{
						std::swap(face.points[1], face.points[2]);
						face.normal = FaceNormal(face);
					}
					if (!(face.normal.x < 1.0f || face.normal.y < 1.0f || face.normal.z < 1.0f))
					{
						std::abort();
					}
					faces.push_back(face);
					OnFaceAdded();
				}
			}
		}
	}
	// combine added face with other one if possible
	void Collider::OnFaceAdded()
	{
		auto& addedFace = faces.back();

		auto expandFaceIdx = [&](Face& face, const glm::vec3 point)
		{
			for (size_t i = 0; i < face.points.size(); i++)
			{
				auto& point1 = points[face.points[i]];
				auto& point2 = points[face.points[(i + 1) % face.points.size()]];
				auto edgeNormal = glm::cross(face.normal, point1 - point2);
				if (Nork::Physics::Sign(edgeNormal, point1, points[face.points[(i + 2) % face.points.size()]]) > 0)
				{
					edgeNormal *= -1;
				}
				if (Nork::Physics::Sign(edgeNormal, point1, point) > 0)
				{
					return (i + 1) % face.points.size();
				}
			}
		};

		auto combine = [&](Face& into, Face& from)
		{
			for (size_t i = 0; i < from.points.size(); i++)
			{
				auto& idx = from.points[i];

				bool common = false;
				for (auto idx2 : into.points)
				{
					if (idx == idx2)
					{
						common = true;
						break;
					}
				}
				if (!common)
				{
					auto& point = points[idx];
					auto pos = expandFaceIdx(into, point);
					into.points.insert(into.points.begin() + pos, idx);
				}
			}
		}; 
		for (size_t j = 0; j < faces.size() - 1; j++)
		{
			auto& face1 = faces[j];
			if (face1.normal == addedFace.normal)
			{
				combine(face1, addedFace);
				return faces.pop_back();
			}
		}
	}
	// for serialization purposes now
	void Collider::AddFace(const Face& face)
	{
		faces.push_back(face);
	}
}