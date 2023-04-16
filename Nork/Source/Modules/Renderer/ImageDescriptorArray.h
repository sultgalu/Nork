#pragma once

#include "Image.h"

namespace Nork::Renderer {

class ImageDescriptorArray
{
public:
	ImageDescriptorArray(std::shared_ptr<Vulkan::DescriptorSet>& descriptorSet, uint32_t descriptorIdx, uint32_t arrSize);
	uint32_t AddImage(std::shared_ptr<Image>& img);
	void RemoveImage(uint32_t idx, bool inFlight = true);
public:
	std::vector<std::shared_ptr<Image>> images;
	std::set<uint32_t> freeImageIdxs;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;
	uint32_t descriptorIdx;
};
}