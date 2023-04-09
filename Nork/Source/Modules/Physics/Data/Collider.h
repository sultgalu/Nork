#pragma once

namespace Nork::Physics
{
	struct Face
	{
		glm::vec3 norm;
		uint32_t vertIdx;
	};
	using index_t = uint32_t;
	struct Edge
	{
		union
		{
			index_t first;
			index_t x;
		};
		union
		{
			index_t second;
			index_t y;
		};

		index_t& operator[](uint32_t idx) const
		{
			return ((index_t*)this)[idx];
		}
	};

	struct Collider
	{
		std::vector<glm::vec3> verts = {};
		std::vector<Edge> edges = {};
		std::vector<Face> faces = {};
		std::vector<std::vector<index_t>> faceVerts = {};
		glm::vec3 center = glm::vec3(0);
		bool isActive = true;

		static Collider Cube();

		// calculates the centroid, sorts the face vertices (cw/ccw?)
		void Update();

		std::vector<uint32_t> SideFacesOfVert(uint32_t vertIdx) const;
		std::vector<uint32_t> EdgesOnFace(uint32_t faceIdx) const;
		std::vector<Edge> Edges(const glm::vec3& vert) const;
		std::pair<const glm::vec3&, const glm::vec3&> Vertices(const Edge& edge) const;
		const glm::vec3& VertFromFace(const Face& face) const;
		const glm::vec3& VertFromFace(uint32_t idx) const;
		const glm::vec3& FirstVertFromEdge(const Edge& edge) const;
		const glm::vec3& SecondVertFromEdge(const Edge& edge) const;
		glm::vec3 EdgeDirection(const Edge& edge) const;
		glm::vec3 EdgeMiddle(const Edge& edge) const;
	private:
		void CalculateCentroid();
		void SortFaceVertices();
	};
}
