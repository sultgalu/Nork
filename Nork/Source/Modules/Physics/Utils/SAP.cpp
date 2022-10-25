#include "pch.h"
#include "SAP.h"

namespace Nork::Physics
{
	std::vector<std::pair<uint32_t, std::pair<float, float>>> SAP::GetMinMaxPairsOnAxis(uint32_t ax)
	{
		std::vector<std::pair<uint32_t, std::pair<float, float>>> res;
		res.reserve(world.colliders.size());

		for (size_t i = 0; i < world.colliders.size(); i++)
		{
			auto& verts = world.colliders[i].verts;
			std::pair<float, float> minMax = { verts[0][ax], verts[0][ax] };

			for (size_t j = 1; j < verts.size(); j++)
			{
				if (verts[j][ax] < minMax.first)
					minMax.first = verts[j][ax];

				if (verts[j][ax] > minMax.second)
					minMax.second = verts[j][ax];
			}
			res.push_back(std::pair(i, minMax));
		}
		return res;
	}
	static std::vector<int> counter;
	static void gen(int n)
	{
		std::ranges::generate(counter, [&n]() mutable { return n++; });
	}
	std::vector<std::pair<uint32_t, AABB>> SAP::GetAABBs()
	{
		static std::vector<std::pair<uint32_t, AABB>> res;
		if (counter.size() < world.colliders.size())
		{
			counter.resize(world.colliders.size());
			gen(0);
		}
		res.resize(world.colliders.size());
		std::for_each_n(std::execution::unseq, counter.begin(), world.colliders.size(), [&](auto i)
			{
				res[i] = (std::pair(i, AABB(world.colliders[i].verts)));
				constexpr float bias = 0.0f;
				res[i].second.min -= glm::vec3(bias);
				res[i].second.max += glm::vec3(bias);
			});
		return res;
	}
	// Right now it produces out-of sync jumping bc of the random order of pairs it gives back (y-collision order)
	std::vector<std::pair<index_t, index_t>> SAP::Get()
	{
		std::vector<std::pair<uint32_t, uint32_t>> res;

		auto aabbs = GetAABBs();
		std::sort(std::execution::unseq, aabbs.begin(), aabbs.end(), [](auto& a, auto& b)
			{
				return a.second.min.x < b.second.min.x;
			});
		
		// aabbs sorted now by x axis
		std::vector<uint32_t> currentInterval = { 0 };
		for (uint32_t i = 1; i < aabbs.size(); i++)
		{
			AABB& aabb1 = aabbs[i].second;
			std::vector<uint32_t> newInterval = { i };
			newInterval.reserve(currentInterval.size() * 1.5f);

			for (uint32_t j = 0; j < currentInterval.size(); j++)
			{
				if (aabbs[i].second.min[0] < aabbs[currentInterval[j]].second.max[0])
				{
					newInterval.push_back(currentInterval[j]);
					AABB& aabb2 = aabbs[currentInterval[j]].second;

					if (aabb1.min[1] < aabb2.min[1])
					{
						if (aabb1.max[1] > aabb2.min[1])
							goto Next;
					}
					else if (aabb1.min[1] < aabb2.max[1])
					{
						goto Next;
					}
					continue;
				Next:
					if (aabb1.min[2] < aabb2.min[2])
					{
						if (aabb1.max[2] > aabb2.min[2])
							goto Add;
					}
					else if (aabb1.min[2] < aabb2.max[2])
					{
						goto Add;
					}
					continue;
				Add:
					res.push_back(std::pair(aabbs[i].first, aabbs[currentInterval[j]].first));
				}
			}
			currentInterval = newInterval;
		}
		return res;
	}
}
