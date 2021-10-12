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
	struct Light
	{
		inline static constexpr auto linears = std::array<float, 10>({ 0.22f, 0.14f, 0.09f, 0.07f, 0.045f, 0.027f, 0.022f, 0.014f, 0.007f, 0.0014f });
		inline static constexpr auto quadratics = std::array<float, 10>({ 0.20f, 0.07f, 0.032f, 0.017f, 0.0075f, 0.0028f, 0.0019f, 0.0007f, 0.0002f, 0.000007f });
	};
}