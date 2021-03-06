#pragma once
#include "CollisionDetection.h"
#include "../Data/World.h"

namespace Nork::Physics
{
	class CollisionDetectionCPU : public CollisionDetection
	{
	public:
		CollisionDetectionCPU(World& world) : world(world) {}
		void UpdateTransforms(std::span<glm::vec3> translate, std::span<glm::quat> quaternions) override;
		void BroadPhase() override;
		void NarrowPhase() override;
	public:
		std::vector<std::pair<index_t, index_t>> broadResults;
		std::vector<CollisionResult> narrowResults;
		virtual std::vector<CollisionResult>& GetResults() override
		{
			return narrowResults;
		}
	private:
		World& world;
	};
}


