#pragma once

#include "Utils/LiveData.h"

namespace Nork::Renderer {

class Settings {
public:
	glm::uvec2 resolution;
	struct Bloom {
		uint32_t mipLevels;
		const uint32_t& maxMipLevels() const; // const& -> directly usable with imgui widgets
		static uint32_t minMipLevels;

		uint32_t gaussianKernelSize;
		static uint32_t maxKernelSize;
		static uint32_t minKernelSize;
		float sigma = 100.f; // higher -> more distribution, if you increase the kernel size increase this too
		bool inlineKernelData;
		bool inlineKernelSize;

		bool useBlitFromDownsampling;
		bool blitLinear;

		glm::vec4 threshold;
		static glm::vec4 thresholdDefault;
		bool inlineThreshold;
	};
	struct PostProcess {
		bool bloom;
		float exposure;
		bool inlineExposure;
	};
	Bloom bloom;
	PostProcess postProcess;
	bool shadows;
	bool deferred;

	static LiveData<Settings>& Instance();
};
}
