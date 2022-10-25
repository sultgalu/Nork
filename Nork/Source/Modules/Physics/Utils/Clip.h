#pragma once

#include "../Data/World.h"

namespace Nork::Physics {
	class Clip
	{
	public:
		static std::vector<glm::vec3> VertsOnEdge(glm::vec3 edgeNorm, glm::vec3 edgeVert, std::span<glm::vec3> pointsToClip);
		static std::vector<glm::vec3> FaceOnPlane(glm::vec3& planeNorm, glm::vec3& planeVert, std::span<uint32_t> faceVerts, std::span<glm::vec3> verts);
		static std::vector<glm::vec3> PointsOnPlane(glm::vec3& planeNorm, glm::vec3& planeVert, const std::vector<glm::vec3>& points);
		static std::vector<glm::vec3> FaceOnFace(const Shape& faceShape, const Shape& faceShape2, const uint32_t faceIdx, const uint32_t faceIdx2, bool clipDepth = true);
		static std::vector<glm::vec3> PointsOnFace(const Shape& faceShape, const uint32_t faceIdx, const std::vector<glm::vec3>& points);
	};
}

