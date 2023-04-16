#include "ImageDescriptorArray.h"

namespace Nork::Renderer {

ImageDescriptorArray::ImageDescriptorArray(
	std::shared_ptr<Vulkan::DescriptorSet>& descriptorSet, uint32_t descriptorIdx, uint32_t arrSize)
	: descriptorSet(descriptorSet), descriptorIdx(descriptorIdx)
{
	for (size_t i = 0; i < arrSize; i++)
	{
		freeImageIdxs.insert(i);
	}
	images.resize(arrSize);
}
uint32_t ImageDescriptorArray::AddImage(std::shared_ptr<Image>& img)
{
	uint32_t idx = *freeImageIdxs.begin();
	freeImageIdxs.erase(freeImageIdxs.begin());

	descriptorSet->Writer()
		.Image(descriptorIdx, *img->view, vk::ImageLayout::eShaderReadOnlyOptimal,
			*img->sampler, vk::DescriptorType::eCombinedImageSampler, idx)
		.Write();
	images[idx] = img;
	return idx;
}
void ImageDescriptorArray::RemoveImage(uint32_t idx, bool inFlight)
{
	if (inFlight) {
		auto image = images[idx];
		Commands::Instance().OnRenderFinished([image]() {});
	}
	images[idx] = nullptr;
	freeImageIdxs.insert(idx);
}
}