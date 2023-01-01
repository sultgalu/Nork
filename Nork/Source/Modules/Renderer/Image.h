#pragma once

#include "Vulkan/Image.h"
#include "MemoryTransfer.h"

namespace Nork::Renderer {

class Image
{
public:
	Image(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlagBits usage, vk::ImageAspectFlagBits aspect,
		vk::PipelineStageFlags2 syncStages = {}, vk::AccessFlags2 syncAccess = {},
		const MemoryFlags& memFlags = { .required = MemoryFlags::eDeviceLocal })
		: syncStages(syncStages), syncAccess(syncAccess)
	{
		img = std::make_shared<Vulkan::Image>(
			Vulkan::ImageCreateInfo(width, height, format, usage));
		layout = vk::ImageLayout::eUndefined;

		auto memreq = img->getMemoryRequirements();
		memory = DeviceMemory(
			MemoryAllocator::Instance().Allocate(memreq, memFlags));
		img->BindMemory(memory.Underlying(), memory.PoolOffset());

		view = std::make_shared<Vulkan::ImageView>(Vulkan::ImageViewCreateInfo(img, aspect));
	}
	void Write(const void* data, vk::DeviceSize size)
	{
		Write(data, size, layout);
	}
	void Write(const void* data, vk::DeviceSize size, vk::ImageLayout newLayout)
	{
		Write(data, size, { img->Width(), img->Height() }, { 0, 0 }, newLayout);
	}
	void Write(const void* data, vk::DeviceSize size, vk::Extent2D extent, vk::Offset2D offset, vk::ImageLayout newLayout)
	{
		if (!(img->createInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
			std::unreachable();
		MemoryTransfer::Instance().UploadToImage(**img, extent, offset, data, size,
			syncStages, syncAccess, newLayout, layout);
		layout = newLayout;
	}
	std::shared_ptr<Vulkan::Image> VkImage()
	{
		return view->createInfo.img;
	}
public:
	std::shared_ptr<Vulkan::Image> img;
	DeviceMemory memory;
	vk::ImageLayout layout;
	vk::PipelineStageFlags2 syncStages;
	vk::AccessFlags2 syncAccess;

	std::shared_ptr<Vulkan::ImageView> view;
	std::shared_ptr<Vulkan::Sampler> sampler;
};

}