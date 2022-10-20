module Nork.Physics;

namespace Nork::Physics
{
	void CollisionDetectionCPU::UpdateTransforms(std::span<glm::vec3> translate, std::span<glm::quat> quaternions)
	{
		world.UpdateTransforms(translate, quaternions);
	}
	void CollisionDetectionCPU::BroadPhase()
	{
		broadResults = SAP(world).Get();
	}
	void CollisionDetectionCPU::NarrowPhase()
	{
		narrowResults.clear();
		for (size_t i = 0; i < broadResults.size(); i++)
		{
			auto res = SAT(world.shapes[broadResults[i].first], world.shapes[broadResults[i].second]).GetResult();
			res.pair = glm::uvec2(broadResults[i].first, broadResults[i].second);
			res.depth *= -1;
			res.dir *= -1;
			narrowResults.push_back(res);
		}
	}
}