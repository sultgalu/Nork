#include "Collider.h"

#include <glm/gtx/vector_angle.hpp>

namespace Nork::Physics {
std::vector<uint32_t> Collider::SideFacesOfVert(uint32_t vertIdx) const
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
std::vector<uint32_t> Collider::EdgesOnFace(uint32_t faceIdx) const
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
std::vector<Edge> Collider::Edges(const glm::vec3& vert) const
{
	std::vector<Edge> result;

	for (size_t i = 0; i < edges.size(); i++)
	{
		if (verts[edges[i].first] == vert || verts[edges[i].second] == vert)
		{
			result.push_back(std::ref(edges[i]));
		}
	}

	return result;
}
std::pair<const glm::vec3&, const glm::vec3&> Collider::Vertices(const Edge& edge) const
{
	return std::pair<const glm::vec3&, const glm::vec3&> { verts[edge[0]], verts[edge[1]] };
}
const glm::vec3& Collider::VertFromFace(const Face& face) const
{
	return verts[face.vertIdx];
}
const glm::vec3& Collider::VertFromFace(uint32_t idx) const
{
	return verts[faces[idx].vertIdx];
}
const glm::vec3& Collider::FirstVertFromEdge(const Edge& edge) const
{
	return verts[edge[0]];
}
const glm::vec3& Collider::SecondVertFromEdge(const Edge& edge) const
{
	return verts[edge[1]];
}
glm::vec3 Collider::EdgeDirection(const Edge& edge) const
{
	return verts[edge[0]] - verts[edge[1]];
}
glm::vec3 Collider::EdgeMiddle(const Edge& edge) const
{
	return (verts[edge[0]] + verts[edge[1]]) / 2.0f;
}
void Collider::CalculateCentroid() {
	SortFaceVertices();
	float sumArea = 0;
	center = glm::vec3(0);
	for (auto& face : faceVerts) {
		auto& a = verts[face[0]];
		for (size_t i = 2; i < face.size(); i++) {
			auto& b = verts[face[i - 1]];
			auto& c = verts[face[i]]; // abc forms a triangle <- b and c are adjacent <- we sorted the face
			auto ab = b - a;
			auto ac = c - a;
			auto area = glm::length(glm::cross(ab, ac)) / 2;
			sumArea += area;
			auto centroid = (a + b + c);
			centroid /= 3;
			center += centroid * area;
		}
	}
	center /= sumArea;
}
void Collider::SortFaceVertices() {
	for (int i = 0; i < faceVerts.size(); i++) {
		auto& face = faceVerts[i];
		const auto& norm = faces[i].norm;
		auto referenceIdx = face[0];
		auto referenceVector = verts[referenceIdx] - verts[face[1]];
		std::sort(face.begin(), face.end(), [&](index_t i0, index_t i1) {
			if (i0 == referenceIdx)
				return false;
			if (i1 == referenceIdx)
				return true;
			auto vec0 = verts[referenceIdx] - verts[i0];
			auto vec1 = verts[referenceIdx] - verts[i1];
			auto cross0 = glm::cross(referenceVector, vec0);
			auto cross1 = glm::cross(referenceVector, vec1);
			auto magSqr0 = glm::dot(cross0, norm);
			auto magSqr1 = glm::dot(cross1, norm);
			return magSqr0 > magSqr1;
		});

		// test correctness
		// for (size_t j = 0; j < face.size(); j++)
		// {
		// 	auto& v1 = verts[face[j]];
		// 	auto& v2 = verts[face[(j + 1) % face.size()]];
		// 	auto edgeNorm = glm::normalize(glm::cross(v2 - v1, norm));
		// 	for (size_t k = 0; k < face.size() - 2; k++)
		// 	{
		// 		auto& v3 = verts[face[(j + 2 + k) % face.size()]];
		// 		auto vec = v3 - v1;
		// 		if (glm::dot(vec, edgeNorm) < 0) {
		// 			std::unreachable();
		// 		}
		// 	}
		// }
	}
}
void Collider::Update()
{
	CalculateVolume();
}

void Collider::CalculateVolume()
{
	CalculateCentroid();
	int faceIdx = -1;
	float volumeSum = 0;
	for (auto& face : faceVerts) {
		++faceIdx;
		auto& a = verts[face[0]];
		for (size_t i = 2; i < face.size(); i++) {
			auto& b = verts[face[i - 1]];
			auto& c = verts[face[i]]; // abc forms a triangle <- b and c are adjacent <- we sorted the face
			auto ab = b - a;
			auto ac = c - a;
			auto area = glm::length(glm::cross(ab, ac)) / 2;
			auto height = glm::dot(faces[faceIdx].norm, a - center);
			volumeSum += area * height / 3;
		}
	}
	volume = volumeSum;
}

Collider Collider::Cube()
{
	constexpr float size = 1;
	constexpr auto scale = glm::vec3(size);

	return Collider{ .verts = {
		glm::vec3(-scale.x, -scale.y, -scale.z),
		glm::vec3(scale.x, -scale.y, -scale.z),
		glm::vec3(scale.x,  scale.y, -scale.z),
		glm::vec3(-scale.x,  scale.y, -scale.z),
		glm::vec3(-scale.x, -scale.y,  scale.z),
		glm::vec3(scale.x, -scale.y,  scale.z),
		glm::vec3(scale.x,  scale.y,  scale.z),
		glm::vec3(-scale.x,  scale.y,  scale.z)
	}, .edges = {
		// Front
		Edge {.x = 0, .y = 1 },
		Edge {.x = 1, .y = 2 },
		Edge {.x = 2, .y = 3 },
		Edge {.x = 3, .y = 0 },
		// Right
		Edge {.x = 1, .y = 5 },
		Edge {.x = 5, .y = 6 },
		Edge {.x = 6, .y = 2 },
		// Left
		Edge {.x = 0, .y = 4 },
		Edge {.x = 4, .y = 7 },
		Edge {.x = 7, .y = 3 },
		// Top
		Edge {.x = 7, .y = 6 },
		// Bottom
		Edge {.x = 4, .y = 5 },
		// Back
	}, .faces = {
		Face {.norm = glm::vec3(0, 0, -1), .vertIdx = 0}, // Front
		Face {.norm = glm::vec3(1, 0, 0), .vertIdx = 1 }, // Right
		Face {.norm = glm::vec3(-1, 0, 0), .vertIdx = 4 }, // Left
		Face {.norm = glm::vec3(0, 1, 0), .vertIdx = 3 }, // Top
		Face {.norm = glm::vec3(0, -1, 0), .vertIdx = 4 }, // Bottom
		Face {.norm = glm::vec3(0, 0, 1), .vertIdx = 5 }  // Back
	}, .faceVerts = {
		{ 0, 1, 2, 3 }, // Front
		{ 1, 5, 6, 2 }, // Right
		{ 4, 0, 3, 7 }, // Left
		{ 3, 2, 6, 7 }, // Top
		{ 4, 5, 1, 0 }, // Bottom
		{ 5, 4, 7, 6 }  // Back
	},
		.center = glm::vec3(0),
		.isActive = true
	};
}
}