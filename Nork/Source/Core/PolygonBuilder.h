#pragma once

#include "Modules/Physics/Data/World.h"

namespace Nork
{
	struct PolygonBase
	{
	public:

	public:
		std::vector<glm::vec3> vertices;
		std::vector<std::array<uint32_t, 2>> edges;
	};
	struct Polygon : PolygonBase
	{
		struct Face
		{
			std::vector<uint32_t> vertices;
			glm::vec3 normal;
		};
		std::vector<Face> faces;
	};
	struct PolygonMesh : PolygonBase
	{
	public:
		std::vector<std::array<uint32_t, 3>> triangles;
	};
	class PolygonBuilder
	{
	public:
		PolygonBuilder() = default;
		PolygonBuilder(const Physics::Collider& coll);
		void Scale(const glm::vec3& scale);

		const Polygon BuildPolygon() const;
		PolygonMesh BuildMesh() const;
		Physics::Collider BuildCollider() const;
	private:
		std::vector<std::array<uint32_t, 3>> TriangulateFace(const Polygon::Face& face) const;
	public:
		std::vector<glm::vec3> vertices;
		std::vector<std::array<uint32_t, 2>> edges;
		glm::vec3 center = glm::zero<glm::vec3>();
	};
}