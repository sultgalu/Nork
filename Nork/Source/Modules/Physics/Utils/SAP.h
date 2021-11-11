#pragma once
#include "../Data/World.h"
#include "AABB.h"

namespace Nork::Physics
{
	class SAP
	{
	public:
		SAP(World& world)
			: world(world)
		{

		}
		std::vector<std::pair<uint32_t, std::pair<float, float>>> GetMinMaxPairsOnAxis(uint32_t ax);
		std::vector<std::pair<uint32_t, AABB>> GetAABBs();
		std::vector<std::pair<uint32_t, uint32_t>> Get();
		World& world;

		static std::vector<std::pair<std::string, float>> GetDeltas();
	};

}

