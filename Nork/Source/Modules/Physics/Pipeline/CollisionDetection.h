#pragma once

#include "../Data/Common.h"

namespace Nork::Physics
{
	class CollisionDetection
	{
	public:
		virtual std::vector<CollisionResult>& GetResults() = 0;
		virtual void UpdateTransforms(std::span<glm::vec3> translate, std::span<glm::quat> quaternions) = 0;
		virtual void BroadPhase() = 0;
		virtual void NarrowPhase() = 0;
	};
}