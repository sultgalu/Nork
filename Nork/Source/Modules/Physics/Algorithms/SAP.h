#pragma once
#include "../Data/Object.h"
#include "AABB.h"

namespace Nork::Physics
{
	class SAP
	{
	public:
		SAP(const std::span<Object> objs)
			: objs(objs)
		{

		}
		std::vector<std::pair<uint32_t, std::pair<float, float>>> GetMinMaxPairsOnAxis(uint32_t ax);
		std::vector<std::pair<uint32_t, AABB>> GetAABBs();
		std::vector<std::pair<uint32_t, uint32_t>> Get();
		const std::span<Object> objs;
	};

}

