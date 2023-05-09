#include "RendererSettings.h"

namespace Nork::Renderer {

static glm::uvec2 Resolution() {
	return Settings::Instance()->resolution;
}

uint32_t Settings::Bloom::minKernelSize = 1;
uint32_t Settings::Bloom::maxKernelSize = 13;
uint32_t Settings::Bloom::minMipLevels = 1;
glm::vec4 Settings::Bloom::thresholdDefault = glm::vec4(0.2126, 0.7152, 0.0722, 1.0);
const uint32_t& Settings::Bloom::maxMipLevels() const {
	static uint32_t val = 0;
	val = static_cast<uint32_t>(std::floor(std::log2(std::max(Resolution().x, Resolution().y)))) + 1;
	return val;
}

static std::unique_ptr<LiveData<Settings>> Create() {
	auto settings = std::make_unique<LiveData<Settings>>();
	*settings = Settings{
		.resolution = glm::uvec2(1920, 1080),

		.bloom = Settings::Bloom{
			.mipLevels = 10,
			.gaussianKernelSize = 9,
			.sigma = 10.f,
			.inlineKernelData = false,
			.inlineKernelSize = false,
			.useBlitFromDownsampling = false,
			.blitLinear = true,
			.threshold = Settings::Bloom::thresholdDefault,
			.inlineThreshold = false
		},
		.postProcess = Settings::PostProcess{
			.bloom = true,
			.exposure = 10,
			.inlineExposure = false
		},
		.shadows = true,
		.deferred = false,
	};

	return settings;
}

LiveData<Settings>& Settings::Instance()
{
	static std::unique_ptr<LiveData<Settings>> instance = Create();
	return *instance;
}
}
