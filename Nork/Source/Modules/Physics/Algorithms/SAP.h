#pragma once
#include "../Data/Object.h"

namespace Nork::Physics
{
class SAP
{
public:
	SAP(const std::span<Object> objs)
		: objs(objs)
	{

	}
	std::vector<std::pair<ColliderIndex, AABB>> GetAABBs();
	std::vector<std::pair<ColliderIndex, ColliderIndex>> Get();
	const std::span<Object> objs;
};

}

