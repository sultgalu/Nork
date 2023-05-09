#include "ShadowMapPass.h"
#include "Vulkan/SwapChain.h"
#include "RendererSettings.h"

namespace Nork::Renderer {

static constexpr vk::Format format = vk::Format::eD32Sfloat;

ShadowMapPass::ShadowMapPass()
{
	CreateRenderPass();
	CreateGraphicsPipeline();

	descriptorSetLayout = std::make_shared<Vulkan::DescriptorSetLayout>(Vulkan::DescriptorSetLayoutCreateInfo()
		.Binding(0, vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eGeometry));
	descriptorPool = std::make_shared<Vulkan::DescriptorPool>(
		Vulkan::DescriptorPoolCreateInfo({ descriptorSetLayout }, 1));
	descriptorSet = std::make_shared<Vulkan::DescriptorSet>(Vulkan::DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayout));
	descriptorSet->Writer()
		.Buffer(0, *Resources::Instance().pShadowVps->Underlying(), 0, 
			Resources::Instance().DynamicSize(*Resources::Instance().pShadowVps), vk::DescriptorType::eUniformBufferDynamic)
		.Write();
	CreateGraphicsPipelineCube();

	auto createInfo = Vulkan::SamplerCreateInfo();
	createInfo.setCompareOp(vk::CompareOp::eLess);
	createInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge);
	createInfo.setAddressModeV(vk::SamplerAddressMode::eClampToEdge);
	createInfo.setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
	sampler = std::make_shared<Vulkan::Sampler>(createInfo);
	samplerCube = std::make_shared<Vulkan::Sampler>(createInfo); // TODO: USE CUBE SPECIFIC SAMPLING
	instance = this;
}
void ShadowMapPass::CreateGraphicsPipeline()
{
	using namespace Vulkan;
	ShaderModule vertShaderModule(LoadShader("Source/Shaders/dirShadMap.vert"), vk::ShaderStageFlagBits::eVertex);
	vk::PushConstantRange vpPush;
	vpPush.size = sizeof(glm::mat4);
	vpPush.stageFlags = vk::ShaderStageFlagBits::eVertex;
	pipelineLayout = std::make_shared<PipelineLayout>(
		PipelineLayoutCreateInfo({ **Resources::Instance().descriptorSetLayout }, { vpPush }));
	pipeline = std::make_shared<Vulkan::Pipeline>(Vulkan::PipelineCreateInfo()
		.Layout(**pipelineLayout)
		.AddShader(vertShaderModule)
		.VertexInput<Data::Vertex>() // could only indlude vertex.pos
		.InputAssembly(vk::PrimitiveTopology::eTriangleList)
		.Rasterization(true) 
		.Multisampling()
		.ColorBlend(0)
		.RenderPass(**renderPass, 0)
		.DepthStencil(true, true, vk::CompareOp::eLess));
}
void ShadowMapPass::CreateGraphicsPipelineCube()
{
	Vulkan::ShaderModule vertShader(LoadShader("Source/Shaders/point_shadow_shader.vert"), vk::ShaderStageFlagBits::eVertex);
	Vulkan::ShaderModule fragShader(LoadShader("Source/Shaders/point_shadow_shader.frag"), vk::ShaderStageFlagBits::eFragment);
	Vulkan::ShaderModule geomShader(LoadShader("Source/Shaders/point_shadow_shader.geom"), vk::ShaderStageFlagBits::eGeometry);
	auto idxPush = vk::PushConstantRange(vk::ShaderStageFlagBits::eGeometry, 0, sizeof(uint32_t));
	auto fragPush = vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment, idxPush.size, sizeof(float) * 3 + sizeof(glm::vec3));

	pipelineLayoutCube = std::make_shared<Vulkan::PipelineLayout>(
		Vulkan::PipelineLayoutCreateInfo({ **Resources::Instance().descriptorSetLayout, **descriptorSetLayout }, 
			{ idxPush, fragPush }));
	pipelineCube = std::make_shared<Vulkan::Pipeline>(Vulkan::PipelineCreateInfo()
		.Layout(**pipelineLayoutCube)
		.AddShader(vertShader)
		.AddShader(fragShader)
		.AddShader(geomShader)
		.VertexInput<Data::Vertex>() // could include only vertex.pos
		.InputAssembly(vk::PrimitiveTopology::eTriangleList)
		.Rasterization(true, vk::FrontFace::eClockwise)
		.Multisampling()
		.ColorBlend(0)
		.RenderPass(**renderPass, 0)
		.DepthStencil(true, true, vk::CompareOp::eLess));
}
void ShadowMapPass::CreateRenderPass()
{
	using namespace Vulkan;
	Vulkan::RenderPassCreateInfo createInfo;

	uint32_t attIdx = 0;
	createInfo.Attachments(std::vector<AttachmentDescription>(1));
	createInfo.attachments[attIdx]
		.setFormat(format)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore);

	uint32_t subPass = 0;
	std::vector<SubpassDescription> subPasses(1);
	subPasses[subPass]
		.DepthAttachment(attIdx);
	createInfo.Subpasses(subPasses);
	createInfo.Dependencies({
		// wait for previous subpass reads
		vk::SubpassDependency(VK_SUBPASS_EXTERNAL, subPass)
		.setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
		.setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
		.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite),
		// make next subpasses wait for writes
		vk::SubpassDependency(subPass, VK_SUBPASS_EXTERNAL)
		.setSrcStageMask(vk::PipelineStageFlagBits::eLateFragmentTests)
		.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
		});
	renderPass = std::make_shared<Vulkan::RenderPass>(createInfo);
}
void ShadowMapPass::RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame)
{
	if (!Settings::Instance()->shadows) {
		return;
	}
	cmd.bindVertexBuffers(0, **Resources::Instance().vertexBuffer->buffer->Underlying(), { 0 });
	cmd.bindIndexBuffer(**Resources::Instance().indexBuffer->buffer->Underlying(), 0, vk::IndexType::eUint32);

	RecordCmdDirectional(cmd, imageIndex, currentFrame);
	RecordCmdPoint(cmd, imageIndex, currentFrame);
}
void ShadowMapPass::RecordCmdDirectional(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame)
{
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **pipelineLayout, 0,
		{ **Resources::Instance().descriptorSet }, // resource dset binding should happen outside
			{
				Resources::Instance().DynamicOffset(*Resources::Instance().drawParams)
			});

	for (auto& shadowMap : Resources::Instance().shadowMaps)
	{
		BeginRenderPass(**renderPass, *shadowMap->fb, cmd);

		cmd.setViewport(0, ViewportFull(*shadowMap->fb));
		cmd.setScissor(0, ScissorFull(*shadowMap->fb));

		cmd.pushConstants<glm::mat4>(**pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, shadowMap->vp);
		cmd.drawIndexedIndirect(**Resources::Instance().drawCommands->Underlying(),
			Resources::Instance().DynamicOffset(*Resources::Instance().drawCommands),
			Resources::Instance().drawCommandCount.defaults + Resources::Instance().drawCommandCount.blend,
			sizeof(vk::DrawIndexedIndirectCommand));

		cmd.endRenderPass();
	}
}
void ShadowMapPass::RecordCmdPoint(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame)
{
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelineCube);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **pipelineLayoutCube, 0,
		{ **Resources::Instance().descriptorSet, **descriptorSet }, 
		{ Resources::Instance().DynamicOffset(*Resources::Instance().drawParams),
		Resources::Instance().DynamicOffset(*Resources::Instance().pShadowVps) });

	uint32_t geomPush = 0;
	for (auto& shadowMap : Resources::Instance().shadowMapsCube)
	{
		BeginRenderPass(**renderPass, *shadowMap->fb, cmd);

		cmd.setViewport(0, ViewportFull(*shadowMap->fb));
		cmd.setScissor(0, ScissorFull(*shadowMap->fb));

		cmd.pushConstants<uint32_t>(**pipelineLayoutCube, vk::ShaderStageFlagBits::eGeometry, 0, geomPush);
		cmd.pushConstants<float>(**pipelineLayoutCube, vk::ShaderStageFlagBits::eFragment, 4, shadowMap->Shadow()->far);
		cmd.pushConstants<glm::vec3>(**pipelineLayoutCube, vk::ShaderStageFlagBits::eFragment, 16, shadowMap->position);
		cmd.drawIndexedIndirect(**Resources::Instance().drawCommands->Underlying(),
			Resources::Instance().DynamicOffset(*Resources::Instance().drawCommands),
			Resources::Instance().drawCommandCount.defaults + Resources::Instance().drawCommandCount.blend,
			sizeof(vk::DrawIndexedIndirectCommand));

		cmd.endRenderPass();
		geomPush++;
	}
}
vk::Format ShadowMapPass::Format()
{
	return format;
}
ShadowMapPass* ShadowMapPass::instance = nullptr;
}
