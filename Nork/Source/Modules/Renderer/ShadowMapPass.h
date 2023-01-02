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
	void CreateRenderPass();
	void RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame) override;
	std::shared_ptr<ShadowMap> CreateShadowMap2D(uint32_t width, uint32_t height);
public:
	std::shared_ptr<Vulkan::RenderPass> renderPass;
	std::shared_ptr<Vulkan::PipelineLayout> pipelineLayout;
	std::shared_ptr<Vulkan::Pipeline> pipeline;
	std::shared_ptr<Vulkan::Sampler> sampler;
public:
	static ShadowMapPass& Instance()
	{
		return *instance;
	}
private:
	static ShadowMapPass* instance;
};

}