#pragma once

#define NORK_ASSERT

namespace Nork::Renderer::Config
{
	struct LightData
	{
		inline static constexpr size_t dirLightsLimit = 10, dirShadowsLimit = 5,
			pointLightsLimit = 10, pointShadowsLimit = 5;
	};
	struct UBOIdx
	{
		inline static constexpr int lights = 0, common = 1;
	};
}