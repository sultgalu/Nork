#pragma once

#include "RenderPass.h"
#include "Image.h"
#include "Resources.h"

namespace Nork::Renderer {

class DeferredPass : public RenderPass
{
public:
	static DeferredPass& Instance();
	DeferredPass();
	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame) override;
	void RefreshShaders() override;
private:
	void CreateDeferredGPassPipeline();
	void CreateDeferredLightPassPipeline();
	void CreateForwardPipeline();
	void CreateLightlessPipeline();
public:

	std::shared_ptr<Vulkan::RenderPass> renderPass;

	std::shared_ptr<Image> depthImage;
	std::shared_ptr<Image> gCol;
	std::shared_ptr<Image> gPos;
	std::shared_ptr<Image> gNorm;
	std::shared_ptr<Image> gMR;
	std::shared_ptr<Image> fbColor;
	std::shared_ptr<Image> emissiveColor;
	std::shared_ptr<Vulkan::Framebuffer> fb;

	std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayoutPP;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSetPP;

	std::shared_ptr<Vulkan::PipelineLayout> pipelineLayout;
	std::shared_ptr<Vulkan::Pipeline> pipelineGPass;
	std::shared_ptr<Vulkan::Pipeline> pipelineLPass;
	std::shared_ptr<Vulkan::Pipeline> pipelineForward;
	std::shared_ptr<Vulkan::Pipeline> pipelineLightless;
};
}