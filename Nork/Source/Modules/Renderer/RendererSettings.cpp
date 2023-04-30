#include "RendererSettings.h"

namespace Nork::Renderer {

static glm::uvec2 Resolution() {
	return *Settings::Instance().resolution;
}

uint32_t Settings::Bloom::minKernelSize = 1;
uint32_t Settings::Bloom::maxKernelSize = (128 - 8 - 4) / sizeof(float);
uint32_t Settings::Bloom::minMipLevels = 1;
const uint32_t& Settings::Bloom::maxMipLevels() const {
	static uint32_t val = 0;
	val = static_cast<uint32_t>(std::floor(std::log2(std::max(Resolution().x, Resolution().y)))) + 1;
	return val;
}

static std::unique_ptr<Settings> Create() {
	auto settings = std::make_unique<Settings>();
	
	settings->resolution = glm::uvec2(1920, 1080);

	settings->bloom = Settings::Bloom{
		.mipLevels = 8,
		.gaussianKernelSize = 5,
		.sigma = 2.f,
		.inlineKernelData = false,
		.inlineKernelSize = false,
		.useBlitFromDownsampling = false,
		.blitLinear = true
	};

	return settings;
}

Settings& Settings::Instance()
{
	static std::unique_ptr<Settings> instance = Create();
	return *instance;
}
}
