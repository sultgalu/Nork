#include "EditorImage.h"

#include "Modules/Renderer/Resources.h"

namespace Nork::Editor {
EditorImage::~EditorImage()
{
	// vkFreeDescriptorSets(*Renderer::Vulkan::Device::Instance(), pool?, 1, (VkDescriptorSet*)&descritptorSet);
}
EditorImage& EditorImage::operator=(const std::shared_ptr<Renderer::Image>& img)
{
	if (!this->img.expired() && Img().view == img->view)
		return *this;

	this->img = img;
	if (sampler.expired())
		sampler = Renderer::Resources::Instance().Textures().defaultSampler;
	if (descritptorSet)
	{
		auto oldDs = descritptorSet;
		Renderer::Commands::Instance().OnRenderFinished([oldDs]()
		{
			ImGui_ImplVulkan_RemoveTexture(oldDs);
		});
	}
	descritptorSet = ImGui_ImplVulkan_AddTexture(*Sampler(), **Img().view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	// else
	// {
	// 	vk::DescriptorImageInfo desc_image = {};
	// 	desc_image.sampler = *Sampler();
	// 	desc_image.imageView = **Img().view;
	// 	desc_image.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	// 	vk::WriteDescriptorSet write = {};
	// 	write.dstSet = descritptorSet;
	// 	write.descriptorCount = 1;
	// 	write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	// 	write.setImageInfo(desc_image);
	// 	Renderer::Vulkan::Device::Instance().updateDescriptorSets(write, {});
	// }
	return *this;
}
Renderer::Image& EditorImage::Img()
{
	return *img.lock();
}
Renderer::Vulkan::Sampler& EditorImage::Sampler()
{
	return *sampler.lock();
}
}