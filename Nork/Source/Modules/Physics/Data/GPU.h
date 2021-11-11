#pragma once

#include "Common.h"

namespace Nork::Physics
{
	struct ShapeGPU
	{
		uint32_t vertStart, vertCount;
		uint32_t edgeStart, edgeCount;
		uint32_t faceStart, faceCount;
	};

	
	struct AABBGPU // TRY TIGHTLY PACK THEM ON GPU
	{
		glm::vec4 min;
		glm::vec4 max;
	};
}