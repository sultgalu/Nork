#pragma once

#include "Modules/Renderer/Image.h"

namespace Nork::Editor {

struct EditorImage
{
	std::weak_ptr<Renderer::Vulkan::Sampler> sampler;
	std::weak_ptr<Renderer::Image> img;
	vk::DescriptorSet descritptorSet = VK_NULL_HANDLE;

	~EditorImage();
	Renderer::Image& Img();
	Renderer::Vulkan::Sampler& Sampler();
	EditorImage& operator=(const std::shared_ptr<Renderer::Image>& img);
	operator bool()
	{
		return descritptorSet;
	}
};
}