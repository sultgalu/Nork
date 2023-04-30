#pragma once

#include "RenderPass.h"
#include "Image.h"
#include "Resources.h"

namespace Nork::Renderer {

class BloomPass : public RenderPass
{
public:
	BloomPass(const std::shared_ptr<Image>& target);
	void CreateTextures();
	void CreatePipelineLayout();
	void CreatePipeline();
	void WriteDescriptorSets();
	void RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame) override;
	void RefreshShaders() override;
public:

	std::shared_ptr<Image> target;
	std::shared_ptr<Image> srcImage;
	std::shared_ptr<Image> pongImage;
	std::vector<std::shared_ptr<Vulkan::ImageView>> srcMipmaps;

	std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayout;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;

	std::shared_ptr<Vulkan::PipelineLayout> pipelineLayout;
	std::shared_ptr<Vulkan::ComputePipeline> pipeline;
};
}