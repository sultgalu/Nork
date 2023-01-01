#pragma once

#include "RenderPass.h"
#include "Vulkan/SwapChain.h"
#include "Image.h"
#include "Data/Vertex.h"
#include "Resources.h"

namespace Nork::Renderer {

class DeferredPass : public RenderPass
{
public:
	DeferredPass();
	void createGraphicsPipeline();
	void createRenderPass();
	void recordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame) override;
public:

	std::shared_ptr<Vulkan::RenderPass> renderPass;

	std::shared_ptr<Image> depthImage;
	std::shared_ptr<Image> gCol;
	std::shared_ptr<Image> gPos;
	std::shared_ptr<Image> gNorm;
	std::shared_ptr<Image> gMR;
	std::shared_ptr<Image> fbColor;
	std::shared_ptr<Vulkan::Framebuffer> fb;

	std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayoutPP;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSetPP;

	std::shared_ptr<Vulkan::PipelineLayout> pipelineLayout;
	std::shared_ptr<Vulkan::Pipeline> pipelineGPass;
	std::shared_ptr<Vulkan::Pipeline> pipelineLPass;
	std::shared_ptr<Vulkan::Pipeline> pipelinePP;
};
}