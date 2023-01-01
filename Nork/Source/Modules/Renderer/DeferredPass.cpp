#include "DeferredPass.h"

namespace Nork::Renderer {

DeferredPass::DeferredPass()
{
	createRenderPass();

	auto w = Vulkan::SwapChain::Instance().Width();
	auto h = Vulkan::SwapChain::Instance().Height();
	using enum vk::ImageUsageFlagBits;
	depthImage = std::make_shared<Image>(w, h, Vulkan::Format::depth32, eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth);
	fbColor = std::make_shared<Image>(w, h, Vulkan::Format::rgba32f, eColorAttachment | eInputAttachment | eTransferSrc | eSampled,
		vk::ImageAspectFlagBits::eColor);
	gPos = std::make_shared<Image>(w, h, Vulkan::Format::rgba32f, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
	gNorm = std::make_shared<Image>(w, h, Vulkan::Format::rgba32f, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
	gMR = std::make_shared<Image>(w, h, Vulkan::Format::rgba32f, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
	gCol = std::make_shared<Image>(w, h, Vulkan::Format::rgba32f, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
	fb = std::make_shared<Vulkan::Framebuffer>(Vulkan::FramebufferCreateInfo(w, h, **renderPass,
		{ gPos->view, gCol->view, gNorm->view, gMR->view, fbColor->view, depthImage->view }));

	using namespace Vulkan;
	descriptorSetLayoutPP = std::make_shared<Vulkan::DescriptorSetLayout>(Vulkan::DescriptorSetLayoutCreateInfo()
		.Binding(0, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment) // gpos
		.Binding(1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment) // gCol
		.Binding(2, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment) // gNorm
		.Binding(3, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment) // gMR
		.Binding(4, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)); // finalColor

	descriptorPool = std::make_shared<Vulkan::DescriptorPool>(
		DescriptorPoolCreateInfo({ descriptorSetLayoutPP }, 1));

	descriptorSetPP = std::make_shared<DescriptorSet>(DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayoutPP));
	createGraphicsPipeline();

	auto textureSampler = std::make_shared<Sampler>();
	descriptorSetPP->Writer()
		.Image(0, *gPos->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Image(1, *gCol->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Image(2, *gNorm->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Image(3, *gMR->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Image(4, *fbColor->view, vk::ImageLayout::eGeneral, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Write();
}
void DeferredPass::createGraphicsPipeline()
{
	using namespace Vulkan;
	ShaderModule vertShaderModule(LoadShader("Source/Shaders/gPass.vert"), vk::ShaderStageFlagBits::eVertex);
	ShaderModule fragShaderModule(LoadShader("Source/Shaders/gPass.frag"), vk::ShaderStageFlagBits::eFragment);
	vk::PushConstantRange vpPush; // gpass
	vpPush.size = sizeof(glm::mat4);
	vpPush.stageFlags = vk::ShaderStageFlagBits::eVertex;
	vk::PushConstantRange viewPosPush; // lpass
	viewPosPush.offset = vpPush.size;
	viewPosPush.size = sizeof(glm::vec3);
	viewPosPush.stageFlags = vk::ShaderStageFlagBits::eFragment;
	pipelineLayout = std::make_shared<PipelineLayout>(
		PipelineLayoutCreateInfo({ **Resources::Instance().descriptorSetLayout,
			**descriptorSetLayoutPP, **Resources::Instance().descriptorSetLayoutLights }, { vpPush, viewPosPush }));
	pipelineGPass = std::make_shared<Vulkan::Pipeline>(Vulkan::PipelineCreateInfo()
		.Layout(**pipelineLayout)
		.AddShader(vertShaderModule)
		.AddShader(fragShaderModule)
		.VertexInput<Data::Vertex>()
		.InputAssembly(vk::PrimitiveTopology::eTriangleList)
		.Rasterization(true) // TRUE
		.Multisampling()
		.ColorBlend(4)
		.RenderPass(**renderPass, 0)
		.DepthStencil(true, true, vk::CompareOp::eLess));

	ShaderModule vertShaderModule3(LoadShader("Source/Shaders/pp.vert"), vk::ShaderStageFlagBits::eVertex);
	ShaderModule fragShaderModule3(LoadShader("Source/Shaders/lightPass.frag"), vk::ShaderStageFlagBits::eFragment);
	pipelineLPass = std::make_shared<Pipeline>(PipelineCreateInfo()
		.Layout(**pipelineLayout)
		.AddShader(vertShaderModule3)
		.AddShader(fragShaderModule3)
		.VertexInputHardCoded()
		.InputAssembly(vk::PrimitiveTopology::eTriangleList)
		.Rasterization(false)
		.Multisampling()
		.ColorBlend(1)
		.RenderPass(**renderPass, 1)
		.DepthStencil(false));

	ShaderModule vertShaderModule2(LoadShader("Source/Shaders/pp.vert"), vk::ShaderStageFlagBits::eVertex);
	ShaderModule fragShaderModule2(LoadShader("Source/Shaders/pp.frag"), vk::ShaderStageFlagBits::eFragment);
	pipelinePP = std::make_shared<Pipeline>(PipelineCreateInfo()
		.Layout(**pipelineLayout)
		.AddShader(vertShaderModule2)
		.AddShader(fragShaderModule2)
		.VertexInputHardCoded()
		.InputAssembly(vk::PrimitiveTopology::eTriangleList)
		.Rasterization(false)
		.Multisampling()
		.ColorBlend(1)
		.RenderPass(**renderPass, 2)
		.DepthStencil(false));
}
void DeferredPass::createRenderPass()
{
	using namespace Vulkan;
	Vulkan::RenderPassCreateInfo createInfo;

	uint32_t gPosAttIdx = 0, gColAttIdx = 1, gNormAttIdx = 2, gMRAttIdx = 3, lPassAttIdx = 4, depthAttIdx = 5;
	createInfo.Attachments(std::vector<AttachmentDescription>(6));
	createInfo.attachments[gPosAttIdx]
		.setFormat(Format::rgba32f)
		.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear);
	createInfo.attachments[gColAttIdx] = createInfo.attachments[gPosAttIdx];
	createInfo.attachments[gNormAttIdx] = createInfo.attachments[gPosAttIdx];
	createInfo.attachments[gMRAttIdx] = createInfo.attachments[gPosAttIdx];
	createInfo.attachments[lPassAttIdx]
		.setFormat(Format::rgba32f)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setStoreOp(vk::AttachmentStoreOp::eStore);
	createInfo.attachments[depthAttIdx]
		.setFormat(Format::depth32)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear);

	uint32_t gPass = 0, lPass = 1, ppPass = 2;
	std::vector<SubpassDescription> subPasses(3);
	subPasses[gPass]
		.ColorAttachments({
			{ gPosAttIdx, vk::ImageLayout::eColorAttachmentOptimal },
			{ gColAttIdx, vk::ImageLayout::eColorAttachmentOptimal },
			{ gNormAttIdx, vk::ImageLayout::eColorAttachmentOptimal },
			{ gMRAttIdx, vk::ImageLayout::eColorAttachmentOptimal }
			})
		.DepthAttachment(depthAttIdx);
	subPasses[lPass]
		.ColorAttachments({ { lPassAttIdx, vk::ImageLayout::eColorAttachmentOptimal } })
		.InputAttachments({
			{ gPosAttIdx, vk::ImageLayout::eShaderReadOnlyOptimal },
			{ gColAttIdx, vk::ImageLayout::eShaderReadOnlyOptimal },
			{ gNormAttIdx, vk::ImageLayout::eShaderReadOnlyOptimal },
			{ gMRAttIdx, vk::ImageLayout::eShaderReadOnlyOptimal }
			});
	subPasses[ppPass]
		.ColorAttachments({ { lPassAttIdx, vk::ImageLayout::eGeneral } })
		.InputAttachments({ { lPassAttIdx, vk::ImageLayout::eGeneral } });
	createInfo.Subpasses(subPasses);

	createInfo.Dependencies({
		vk::SubpassDependency(gPass, lPass)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion),

		vk::SubpassDependency(lPass, ppPass)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion),

		vk::SubpassDependency(ppPass, VK_SUBPASS_EXTERNAL)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
		});
	renderPass = std::make_shared<Vulkan::RenderPass>(createInfo);
}
void DeferredPass::recordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame)
{
	using namespace Vulkan;
	BeginRenderPass(**renderPass, *fb, cmd);
	cmd.setViewport(0, ViewportFull(*fb));
	cmd.setScissor(0, ScissorFull(*fb));

	cmd.pushConstants<glm::mat4>(**pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, Resources::Instance().vp);
	cmd.pushConstants<glm::vec3>(**pipelineLayout, vk::ShaderStageFlagBits::eFragment, sizeof(glm::mat4), Resources::Instance().viewPos);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelineGPass);

	// resource dset binding should happen outside
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **pipelineLayout, 0,
		{ **Resources::Instance().descriptorSet, **descriptorSetPP, **Resources::Instance().descriptorSetLights },
			{
				Resources::Instance().DynamicOffset(*Resources::Instance().drawParams),
				Resources::Instance().DynamicOffset(*Resources::Instance().dirLightParams),
				Resources::Instance().DynamicOffset(*Resources::Instance().pointLightParams),
			}
			);

	cmd.bindVertexBuffers(0, **Resources::Instance().vertexBuffer->buffer->Underlying(), { 0 });
	cmd.bindIndexBuffer(**Resources::Instance().indexBuffer->buffer->Underlying(), 0, vk::IndexType::eUint32);
	cmd.drawIndexedIndirect(**Resources::Instance().drawCommands->Underlying(),
		Resources::Instance().DynamicOffset(*Resources::Instance().drawCommands),
		Resources::Instance().drawCommandCount, sizeof(vk::DrawIndexedIndirectCommand));

	cmd.nextSubpass(vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelineLPass);
	cmd.DrawQuad();

	cmd.nextSubpass(vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelinePP);
	cmd.DrawQuad();

	cmd.endRenderPass();
}
}