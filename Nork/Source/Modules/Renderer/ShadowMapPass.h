#pragma once

#include "RenderPass.h"
#include "Image.h"
#include "Resources.h"

namespace Nork::Renderer {

class ShadowMapPass : public RenderPass
{
public:
	ShadowMapPass();
	void CreateGraphicsPipeline();
	void CreateGraphicsPipelineCube();
	void CreateRenderPass();
	void RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame) override;
	void RecordCmdDirectional(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame);
	void RecordCmdPoint(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame);
	vk::Format Format();
public:
	std::shared_ptr<Vulkan::RenderPass> renderPass;
	std::shared_ptr<Vulkan::PipelineLayout> pipelineLayout;
	std::shared_ptr<Vulkan::Pipeline> pipeline;
	std::shared_ptr<Vulkan::Sampler> sampler;

	std::shared_ptr<Vulkan::PipelineLayout> pipelineLayoutCube;
	std::shared_ptr<Vulkan::Pipeline> pipelineCube;
	std::shared_ptr<Vulkan::Sampler> samplerCube;
	std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
	std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayout;
	std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;
public:
	static ShadowMapPass& Instance()
	{
		return *instance;
	}
private:
	static ShadowMapPass* instance;
};

}