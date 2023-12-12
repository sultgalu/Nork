#include "SAP.h"

namespace Nork::Physics
{
static std::vector<uint32_t> counter;
static void gen(uint32_t n)
{
	std::ranges::generate(counter, [&n]() mutable { return n++; });
}
std::vector<std::pair<ColliderIndex, AABB>> SAP::GetAABBs()
{
	static std::vector<std::pair<ColliderIndex, AABB>> res;
	static std::vector<uint32_t> offsets;
	offsets.clear();

	uint32_t collCount = 0;
	for (auto& obj : objs) {
		offsets.push_back(collCount);
		collCount += obj.colliders.size();
	}
	if (counter.size() < collCount)
	{
		counter.resize(collCount);
		gen(0);
	}
	res.resize(collCount);
	std::for_each_n(std::execution::par_unseq, counter.begin(), objs.size(), [&](uint32_t i)
	{
		for (uint32_t j = 0; j < objs[i].colliders.size(); j++) {
			auto& result = res[offsets[i] + j];
			result = std::pair(ColliderIndex{ .objIdx = i, .collIdx = j }, objs[i].colliders[j].aabb);
			constexpr float bias = 0.0f;
			result.second.min -= glm::vec3(bias);
			result.second.max += glm::vec3(bias);
		}
	});
	return res;
}
// Right now it produces out-of sync jumping bc of the random order of pairs it gives back (y-collision order)
std::vector<std::pair<ColliderIndex, ColliderIndex>> SAP::Get()
{
	std::vector<std::pair<ColliderIndex, ColliderIndex>> res;

	auto aabbs = GetAABBs();
	std::sort(std::execution::par_unseq, aabbs.begin(), aabbs.end(), [](auto& a, auto& b)
	{
		return a.second.min.x < b.second.min.x;
	});

	// aabbs sorted now by x axis
	static std::vector<uint32_t> currentInterval;
	currentInterval.clear();
	currentInterval.push_back(0);
	for (uint32_t i = 1; i < aabbs.size(); i++)
	{
		AABB& aabb1 = aabbs[i].second;
		static std::vector<uint32_t> newInterval;
		newInterval.clear();
		newInterval.push_back(i);

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
		std::swap(currentInterval, newInterval);
	}
	return res;
}
}
