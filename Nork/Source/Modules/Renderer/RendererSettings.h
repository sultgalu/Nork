#pragma once

#include "Utils/LiveData.h"

namespace Nork::Renderer {

class Settings {
public:
	LiveData<glm::uvec2> resolution;
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
	};
	LiveData<Bloom> bloom;

	static Settings& Instance();
};
}
