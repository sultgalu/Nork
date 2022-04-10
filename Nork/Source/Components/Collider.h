#pragma once

namespace Nork::Components {
	struct Collider
	{
		using Edge = std::pair<uint32_t, uint32_t>;
		struct Face
		{
			std::vector<uint32_t> points;
			glm::vec3 normal;
		};

		static Collider Cube();

		uint32_t TriangleCount() const;
		std::vector<uint32_t> TriangleIndices() const;
		std::vector<uint32_t> EdgeIndices() const;

		uint32_t AddPoint(const glm::vec3&);
		uint32_t AddEdge(uint32_t, uint32_t);
		void OnEdgeAdded(uint32_t);
		void OnFaceAdded();
		void BuildTriangleFaces();
		void CombineFaces();
		glm::vec3 Center();
		
		auto& Points() const { return points; }
		auto& Faces() const { return faces; }
		auto& Edges() const { return edges; }
	private:
		std::vector<glm::vec3> points;
		std::vector<Edge> edges; // first < second
		std::vector<Face> faces;
	private:
		glm::vec3 FaceNormal(const Face&);
	};
}