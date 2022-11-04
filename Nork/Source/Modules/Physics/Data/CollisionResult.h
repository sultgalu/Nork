#pragma once

namespace Nork::Physics
{
	enum class CollisionType : uint32_t
	{
		FaceVert, VertFace, EdgeEdge
	};

	struct CollisionResult
	{
		glm::vec3 dir;
		float depth;
		CollisionType type;
		uint16_t featureIdx1;
		uint16_t featureIdx2;
	};
}
