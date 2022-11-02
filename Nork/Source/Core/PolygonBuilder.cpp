#include "PolygonBuilder.h"

namespace Nork
{
	PolygonBuilder::PolygonBuilder(const Physics::Collider& coll)
	{
		center = coll.center;
		vertices = coll.verts;
		edges.resize(coll.edges.size());
		std::transform(coll.edges.begin(), coll.edges.end(), edges.begin(), [&](const Physics::Edge& edge)
			{
				return std::array<uint32_t, 2> { edge.first, edge.second };
			});
	}
	void PolygonBuilder::Scale(const glm::vec3& scale)
	{
		for (size_t i = 0; i < vertices.size(); i++)
		{
			vertices[i] *= scale;
		}
	}
	Physics::Collider PolygonBuilder::BuildCollider() const
	{
		auto collider = Physics::Collider { .verts = vertices };
		auto poly = BuildPolygon();
		collider.edges.resize(poly.edges.size());
		std::transform(poly.edges.begin(), poly.edges.end(), collider.edges.begin(), [&](const auto& edge)
			{
				return Physics::Edge{ .first = edge[0], .second = edge[1] };
			});
		collider.faces.resize(poly.faces.size());
		std::transform(poly.faces.begin(), poly.faces.end(), collider.faces.begin(), [&](const Polygon::Face& face)
			{
				return Physics::Face{ .norm = face.normal, .vertIdx = face.vertices[0] };
			});
		collider.faceVerts.resize(poly.faces.size());
		std::transform(poly.faces.begin(), poly.faces.end(), collider.faceVerts.begin(), [&](const Polygon::Face& face)
			{
				return face.vertices;
			});

		return collider;
	}
	PolygonMesh PolygonBuilder::BuildMesh() const
	{
		PolygonMesh mesh;
		auto poly = BuildPolygon();
		mesh.vertices = poly.vertices;
		mesh.edges = poly.edges;
		for (auto& face : poly.faces)
		{
			auto tris = TriangulateFace(face);
			mesh.triangles.insert(mesh.triangles.begin(), tris.begin(), tris.end());
		}

		return mesh;
	}
	struct FaceHelper
	{
		std::unordered_set<uint32_t> vertices;
		std::unordered_set<uint32_t> edges;
		glm::vec3 normal;
	};
	const Polygon PolygonBuilder::BuildPolygon() const
	{
		Polygon poly;
		std::unordered_set<uint32_t> polyVerts;
		std::unordered_set<uint32_t> polyEdges;

		std::vector<FaceHelper> faces;
		auto addEdges = [&](uint32_t commonVert, uint32_t vert1, uint32_t vert2)
		{
		};
		using Edge = std::array<uint32_t, 2>;
		auto connected = [&](const Edge& edge1, const Edge& edge2, uint32_t& vert)
		{
			for (size_t i = 0; i < 2; i++)
				for (size_t j = 0; j < 2; j++)
					if (edge1[i] == edge2[j])
					{
						vert = edge1[i];
						return true;
					}
			return false;
		};

		for (uint32_t edge1Idx = 0; edge1Idx < edges.size() - 1; edge1Idx++)
		{ // build faces from edges
			for (uint32_t edge2Idx = edge1Idx + 1; edge2Idx < edges.size(); edge2Idx++)
			{
				const Edge& edge1 = edges[edge1Idx], edge2 = edges[edge2Idx];
				uint32_t commonVert = 0;
				if (connected(edge1, edge2, commonVert))
				{ // if edges are not connected, they can't be part of the same face
					uint32_t vert1 = edge1[0] != commonVert ? edge1[0] : edge1[1];
					uint32_t vert2 = edge2[0] != commonVert ? edge2[0] : edge2[1];

					auto normal = glm::normalize(glm::cross(vertices[vert1] - vertices[commonVert], vertices[vert2] - vertices[commonVert]));
					if (glm::dot(normal, vertices[commonVert] - center) < 0)
						normal *= -1;
					float distanceFromCenter = glm::dot(normal, vertices[vert1] - center);
					bool onSurface = true;
					for (auto& vert : vertices)
					{
						if (glm::dot(normal, vert - center) - distanceFromCenter > 0.0001f)
						{
							onSurface = false;
							break;
						}
					}
					if (onSurface)
					{
						bool found = false;
						for (auto& face : faces)
						{
							if (glm::dot(face.normal, normal) > 0.999f)
							{
								face.vertices.insert({ commonVert, vert1, vert2 });
								face.edges.insert({ edge1Idx, edge2Idx });
								found = true;
								break;
							}
						}
						if (!found)
						{
							faces.push_back(FaceHelper{ .vertices = { commonVert, vert1, vert2 }, .edges = {edge1Idx, edge2Idx}, .normal = normal });
						}
					}
				}
			}
		}
		for (auto& face : faces)
		{
			glm::vec3 faceCenter = glm::vec3(0);
			for (auto vertIdx : face.vertices)
				faceCenter += vertices[vertIdx];
			faceCenter /= face.vertices.size();

			std::unordered_set<uint32_t> filteredEdges;
			for (auto edgeIdx : face.edges)
			{
				auto& edge = edges[edgeIdx];
				auto edgeVec = vertices[edge[0]] - vertices[edge[1]];
				auto edgeNorm = glm::cross(edgeVec, face.normal); // not normalized
				if (glm::dot(edgeNorm, vertices[edge[0]] - faceCenter) < 0)
					edgeNorm *= -1;

				bool edgeIsInsideFace = false;
				for (auto vertIdx : face.vertices)
				{
					if (vertIdx != edge[0] && vertIdx != edge[1] 
						&& glm::dot(edgeNorm, vertices[edge[0]] - vertices[vertIdx]) < 0)
					{ // vertIdx is outside of edge
						edgeIsInsideFace = true;
						break;
					}
				}
				if (!edgeIsInsideFace)
					filteredEdges.insert(edgeIdx);
			}
			face.vertices.clear();
			for (auto& edgeIdx : filteredEdges)
			{
				face.vertices.insert({ edges[edgeIdx][0], edges[edgeIdx][1] });
			}

			polyEdges.insert(filteredEdges.begin(), filteredEdges.end());
			polyVerts.insert(face.vertices.begin(), face.vertices.end());
			poly.faces.push_back(Polygon::Face { .normal = face.normal });
			for (auto vertIdx : face.vertices)
				poly.faces.back().vertices.push_back(vertIdx);
		}
		poly.edges.reserve(polyEdges.size());
		poly.vertices.reserve(polyVerts.size());
		for (auto idx : polyEdges)
			poly.edges.push_back(edges[idx]);
		for (auto idx : polyVerts)
			poly.vertices.push_back(vertices[idx]);

		return poly;
	}
	std::vector<std::array<uint32_t, 3>> PolygonBuilder::TriangulateFace(const Polygon::Face& face) const
	{
		if (face.vertices.size() < 3) return {};
		uint32_t pivot = face.vertices[0];
		uint32_t current, next;

		auto find_next = [&](uint32_t curr, uint32_t exception)
		{
			auto it = std::find_if(edges.begin(), edges.end(), [&](auto edge)
				{
					if (edge[1] == curr)
						std::swap(edge[0], edge[1]);
					return edge[0] == curr && edge[1] != exception
						&& std::find(face.vertices.begin(), face.vertices.end(), edge[1]) != face.vertices.end();
				});
			if (it == edges.end())
				return exception;
			auto& nextEdge = *it;
			return nextEdge[0] == curr ? nextEdge[1] : nextEdge[0];
		};

		next = find_next(pivot, (uint32_t)-1);
		current = pivot;
		std::vector<std::array<uint32_t, 3>> res;
		while (true)
		{
			auto prev = current;
			current = next;
			next = find_next(current, prev);
			if (next == prev) // face not connected
				return {};
			if (next == pivot)
				break;
			res.insert(res.end(), { pivot, current, next });
		}
		//check for clockwise
		auto& tri = res[0];
		auto norm = glm::cross(vertices[tri[1]] - vertices[tri[0]], vertices[tri[2]] - vertices[tri[0]]);
		if (glm::dot(norm, face.normal) < 0)
		{
			for (auto& tri : res)
			{
				std::swap(tri[0], tri[2]);
			}
		}

		return res;
	}
}