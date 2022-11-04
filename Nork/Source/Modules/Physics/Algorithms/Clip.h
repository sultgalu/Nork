#pragma once

#include "../Data/Collider.h"

namespace Nork::Physics {
	class Clip
	{
	public:
		static std::vector<glm::vec3> VertsOnEdge(const glm::vec3 edgeNorm, const glm::vec3 edgeVert, const std::vector<glm::vec3>& pointsToClip);
		static std::vector<glm::vec3> FaceOnPlane(const glm::vec3& planeNorm, const glm::vec3& planeVert, const std::vector<uint32_t>& faceVerts, const std::vector<glm::vec3>& verts);
		static std::vector<glm::vec3> PointsOnPlane(const glm::vec3& planeNorm, const glm::vec3& planeVert, const std::vector<glm::vec3>& points);
		static std::vector<glm::vec3> FaceOnFace(const Collider& facecollider, const Collider& facecollider2, const uint32_t faceIdx, const uint32_t faceIdx2, bool clipDepth = true);
		static std::vector<glm::vec3> PointsOnFace(const Collider& facecollider, const uint32_t faceIdx, const std::vector<glm::vec3>& points);
	};
}

