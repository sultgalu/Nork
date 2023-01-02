#include "ShadowMapPass.h"
#include "Vulkan/SwapChain.h"

namespace Nork::Renderer {

static constexpr vk::Format format = vk::Format::eD32Sfloat;

ShadowMapPass::ShadowMapPass()
{
	CreateRenderPass();
	CreateGraphicsPipeline();
	auto createInfo = Vulkan::SamplerCreateInfo();
	
	createInfo.setCompareOp(vk::CompareOp::eLess);
	sampler = std::make_shared<Vulkan::Sampler>(createInfo);
	instance = this;
}
void ShadowMapPass::CreateGraphicsPipeline()
{
	using namespace Vulkan;
	ShaderModule vertShaderModule(LoadShader("Source/Shaders/dirShadMap.vert"), vk::ShaderStageFlagBits::eVertex);
	vk::PushConstantRange vpPush; // gpass
	vpPush.size = sizeof(glm::mat4);
	vpPush.stageFlags = vk::ShaderStageFlagBits::eVertex;
	pipelineLayout = std::make_shared<PipelineLayout>(
		PipelineLayoutCreateInfo({ **Resources::Instance().descriptorSetLayout }, { vpPush }));
	pipeline = std::make_shared<Vulkan::Pipeline>(Vulkan::PipelineCreateInfo()
		.Layout(**pipelineLayout)
		.AddShader(vertShaderModule)
		.VertexInput<Data::Vertex>() // could only indlude vertex.pos
		.InputAssembly(vk::PrimitiveTopology::eTriangleList)
		.Rasterization(true) // TRUE
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
	auto depthStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	auto depthAccess = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	createInfo.Dependencies({
		// wait for previous frame to complete writes
		vk::SubpassDependency(VK_SUBPASS_EXTERNAL, subPass)
		.setSrcStageMask(depthStage)
		.setSrcAccessMask(depthAccess)
		.setDstStageMask(depthStage)
		.setDstAccessMask(depthAccess),
		// wait for shadow generation in light pass
		vk::SubpassDependency(subPass, VK_SUBPASS_EXTERNAL)
		.setSrcStageMask(depthStage)
		.setSrcAccessMask(depthAccess)
		.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
		});
	renderPass = std::make_shared<Vulkan::RenderPass>(createInfo);
}
void ShadowMapPass::RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame)
{
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline); // move stuff outside of renderpass
	// resource dset binding should happen outside
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **pipelineLayout, 0,
		{ **Resources::Instance().descriptorSet },
			{
				Resources::Instance().DynamicOffset(*Resources::Instance().drawParams)
			}
			);
	cmd.bindVertexBuffers(0, **Resources::Instance().vertexBuffer->buffer->Underlying(), { 0 });
	cmd.bindIndexBuffer(**Resources::Instance().indexBuffer->buffer->Underlying(), 0, vk::IndexType::eUint32);

	for (auto& shadowMap : Resources::Instance().shadowMaps)
	{
		BeginRenderPass(**renderPass, *shadowMap->fb, cmd);

		cmd.setViewport(0, ViewportFull(*shadowMap->fb));
		cmd.setScissor(0, ScissorFull(*shadowMap->fb));

		cmd.pushConstants<glm::mat4>(**pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, shadowMap->vp);
		cmd.drawIndexedIndirect(**Resources::Instance().drawCommands->Underlying(),
			Resources::Instance().DynamicOffset(*Resources::Instance().drawCommands),
			Resources::Instance().drawCommandCount, sizeof(vk::DrawIndexedIndirectCommand));

		cmd.endRenderPass();
	}
}
std::shared_ptr<ShadowMap> ShadowMapPass::CreateShadowMap2D(uint32_t width, uint32_t height)
{
	if (format != vk::Format::eD16Unorm && format != vk::Format::eD16UnormS8Uint && format != vk::Format::eD24UnormS8Uint &&
		format != vk::Format::eD32Sfloat && format != vk::Format::eD32SfloatS8Uint)
		std::unreachable(); // shadow map image should be in depth format

	auto shadowMap = std::make_shared<ShadowMap>();
	shadowMap->image = std::make_shared<Image>(width, height, format, 
		vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::ImageAspectFlagBits::eDepth);
	shadowMap->image->sampler = sampler;
	shadowMap->fb = std::make_shared<Vulkan::Framebuffer>(Vulkan::FramebufferCreateInfo(
		width, height, **renderPass, { shadowMap->image->view }));
	return shadowMap;
}
ShadowMapPass* ShadowMapPass::instance = nullptr;
}
