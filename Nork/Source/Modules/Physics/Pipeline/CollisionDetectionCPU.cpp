#include "pch.h"
#include "CollisionDetectionCPU.h"
#include "../Utils/SAT.h"
#include "../Utils/SAP.h"
#include <execution>

namespace Nork::Physics
{
	static std::vector<int> counter;
	static void gen(int n)
	{
		std::ranges::generate(counter, [&n]() mutable { return n++; });
	}

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
		if (counter.size() < broadResults.size())
		{
			counter.resize(broadResults.size());
			gen(0);
		}
		narrowResults.resize(broadResults.size());
		std::for_each_n(std::execution::par_unseq, counter.begin(), broadResults.size(), [&](auto i)
			{
				auto res = SAT(world.shapes[broadResults[i].first], world.shapes[broadResults[i].second]).GetResult();
				res.pair = glm::uvec2(broadResults[i].first, broadResults[i].second);
				res.depth *= -1;
				res.dir *= -1;
				narrowResults[i] = res;
			});
	}
}