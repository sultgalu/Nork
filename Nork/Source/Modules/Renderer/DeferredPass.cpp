#include "DeferredPass.h"

#include "Vulkan/SwapChain.h"
#include "Data/Vertex.h"
#include "RendererSettings.h"

namespace Nork::Renderer {

namespace Formats {
static constexpr vk::Format depth = Vulkan::Format::depth32;
static constexpr vk::Format pos = Vulkan::Format::rgba32f;
static constexpr vk::Format norm = Vulkan::Format::rgba16Snorm;

static constexpr vk::Format baseCol = Vulkan::Format::rgba32f;
static constexpr vk::Format final = Vulkan::Format::rgba32f;

static constexpr vk::Format metRough = Vulkan::Format::rgba16f;
}

DeferredPass::DeferredPass()
{
	CreateRenderPass();

	auto w = Settings::Instance().resolution->x;
	auto h = Settings::Instance().resolution->y;
	using enum vk::ImageUsageFlagBits;
	depthImage = std::make_shared<Image>(w, h, Formats::depth, eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth);
	fbColor = std::make_shared<Image>(w, h, Formats::final, eColorAttachment | eInputAttachment | eTransferSrc | eSampled | eStorage,
		vk::ImageAspectFlagBits::eColor);
	gPos = std::make_shared<Image>(w, h, Formats::pos, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
	gCol = std::make_shared<Image>(w, h, Formats::baseCol, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
	gNorm = std::make_shared<Image>(w, h, Formats::norm, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
	gMR = std::make_shared<Image>(w, h, Formats::metRough, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
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
	CreateGraphicsPipeline();

	auto textureSampler = std::make_shared<Sampler>();
	descriptorSetPP->Writer()
		.Image(0, *gPos->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Image(1, *gCol->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Image(2, *gNorm->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Image(3, *gMR->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Image(4, *fbColor->view, vk::ImageLayout::eGeneral, *textureSampler, vk::DescriptorType::eInputAttachment)
		.Write();
}
void DeferredPass::CreateGraphicsPipeline()
{
	using namespace Vulkan;
	vk::PushConstantRange vpPush; // gpass
	vpPush.size = sizeof(glm::mat4);
	vpPush.stageFlags = vk::ShaderStageFlagBits::eVertex;
	vk::PushConstantRange viewPosPush; // lpass
	viewPosPush.offset = vpPush.size;
	viewPosPush.size = sizeof(glm::vec3) + sizeof(float);
	viewPosPush.stageFlags = vk::ShaderStageFlagBits::eFragment;
	pipelineLayout = std::make_shared<PipelineLayout>(
		PipelineLayoutCreateInfo({ **Resources::Instance().descriptorSetLayout,
			**descriptorSetLayoutPP, **Resources::Instance().descriptorSetLayoutLights }, { vpPush, viewPosPush }));
	CreateDeferredGPassPipeline();
	CreateDeferredLightPassPipeline();
	CreateForwardPipeline();
}
void DeferredPass::CreateRenderPass()
{
	using namespace Vulkan;
	Vulkan::RenderPassCreateInfo createInfo;

	uint32_t gPosAttIdx = 0, gColAttIdx = 1, gNormAttIdx = 2, gMRAttIdx = 3, lPassAttIdx = 4, depthAttIdx = 5;
	createInfo.Attachments(std::vector<AttachmentDescription>(6));
	createInfo.attachments[gPosAttIdx]
		.setFormat(Formats::pos)
		.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear);
	createInfo.attachments[gColAttIdx] = createInfo.attachments[gPosAttIdx];
	createInfo.attachments[gNormAttIdx] = createInfo.attachments[gPosAttIdx];
	createInfo.attachments[gMRAttIdx] = createInfo.attachments[gPosAttIdx];
	createInfo.attachments[gColAttIdx].setFormat(Formats::baseCol);
	createInfo.attachments[gNormAttIdx].setFormat(Formats::norm);
	createInfo.attachments[gMRAttIdx].setFormat(Formats::metRough);
	createInfo.attachments[lPassAttIdx]
		.setSamples(vk::SampleCountFlagBits::e1)
		.setFormat(Formats::final)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setStoreOp(vk::AttachmentStoreOp::eStore);
	createInfo.attachments[depthAttIdx]
		.setSamples(vk::SampleCountFlagBits::e1)
		.setFormat(Format::depth32)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear);

	uint32_t gPass = 0, lPass = 1, fPass = 2;
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
	subPasses[fPass]
		.ColorAttachments({ { lPassAttIdx, vk::ImageLayout::eColorAttachmentOptimal } })
		.DepthAttachment(depthAttIdx);
	createInfo.Subpasses(subPasses);

	createInfo.Dependencies({
		vk::SubpassDependency(gPass, lPass)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion),

		vk::SubpassDependency(lPass, fPass)
		.setSrcStageMask(vk::PipelineStageFlagBits::eLateFragmentTests)
		.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
		.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
		});
	renderPass = std::make_shared<Vulkan::RenderPass>(createInfo);
}
void DeferredPass::RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame)
{
	bool deferred = *Settings::Instance().deferred;

	using namespace Vulkan;
	BeginRenderPass(**renderPass, *fb, cmd);
	cmd.setViewport(0, ViewportFull(*fb));
	cmd.setScissor(0, ScissorFull(*fb));

	cmd.pushConstants<glm::mat4>(**pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, Resources::Instance().vp);
	cmd.pushConstants<glm::vec3>(**pipelineLayout, vk::ShaderStageFlagBits::eFragment, sizeof(glm::mat4), Resources::Instance().viewPos);
	cmd.pushConstants<float>(**pipelineLayout, vk::ShaderStageFlagBits::eFragment, sizeof(glm::mat4) + sizeof(glm::vec3), 0.0);

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
	if (deferred) {
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelineGPass);
		cmd.drawIndexedIndirect(**Resources::Instance().drawCommands->Underlying(),
			Resources::Instance().DynamicOffset(*Resources::Instance().drawCommands),
			Resources::Instance().drawCommandCount.defaults, sizeof(vk::DrawIndexedIndirectCommand));
	}

	cmd.nextSubpass(vk::SubpassContents::eInline);
	if (deferred) {
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelineLPass);
		cmd.DrawQuad();
	}

	cmd.nextSubpass(vk::SubpassContents::eInline);
	if (!deferred) {
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelineForward);
		cmd.drawIndexedIndirect(**Resources::Instance().drawCommands->Underlying(),
			Resources::Instance().DynamicOffset(*Resources::Instance().drawCommands),
			Resources::Instance().drawCommandCount.defaults, sizeof(vk::DrawIndexedIndirectCommand));
	}
	if (Resources::Instance().drawCommandCount.blend > 0) {
		cmd.pushConstants<float>(**pipelineLayout, vk::ShaderStageFlagBits::eFragment, sizeof(glm::mat4) + sizeof(glm::vec3), 1.0);
		if (deferred) {
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelineForward);
		}
		cmd.drawIndexedIndirect(**Resources::Instance().drawCommands->Underlying(),
			Resources::Instance().DynamicOffset(*Resources::Instance().drawCommands) + Resources::Instance().drawCommandCount.defaults * sizeof(vk::DrawIndexedIndirectCommand),
			Resources::Instance().drawCommandCount.blend, sizeof(vk::DrawIndexedIndirectCommand));
	}

	cmd.endRenderPass();
}
void DeferredPass::CreateDeferredGPassPipeline()
{
	using namespace Vulkan;
	ShaderModule vertShaderModule(LoadShader("Source/Shaders/gPass.vert"), vk::ShaderStageFlagBits::eVertex);
	ShaderModule fragShaderModule(LoadShader("Source/Shaders/gPass.frag"), vk::ShaderStageFlagBits::eFragment);
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
}
void DeferredPass::CreateDeferredLightPassPipeline()
{
	using namespace Vulkan;
	ShaderModule vertShaderModule3(LoadShader("Source/Shaders/quad.vert"), vk::ShaderStageFlagBits::eVertex);
	ShaderModule fragShaderModule3(LoadShader("Source/Shaders/lightPass.frag", { {"DEFERRED", ""} }), vk::ShaderStageFlagBits::eFragment);
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
}
void DeferredPass::CreateForwardPipeline()
{
	using namespace Vulkan;
	ShaderModule vertShaderModule4(LoadShader("Source/Shaders/gPass.vert", {}, 1), vk::ShaderStageFlagBits::eVertex);
	ShaderModule fragShaderModule4(LoadShader("Source/Shaders/lightPass.frag", { {"FORWARD", ""} }, 1), vk::ShaderStageFlagBits::eFragment);
	pipelineForward = std::make_shared<Pipeline>(PipelineCreateInfo()
		.Layout(**pipelineLayout)
		.AddShader(vertShaderModule4)
		.AddShader(fragShaderModule4)
		.VertexInput<Data::Vertex>()
		.InputAssembly(vk::PrimitiveTopology::eTriangleList)
		.Rasterization(true)
		.Multisampling(vk::SampleCountFlagBits::e1) // TODO dynamic??
		.ColorBlend(1, true) // TODO enable 
		.RenderPass(**renderPass, 2)
		.DepthStencil(true, true, vk::CompareOp::eLess));
}
void DeferredPass::RefreshShaders()
{
	// if (IsShaderSourceChanged("Source/Shaders/bloom.comp", GetMacros())) {
	// 	CreatePipeline();
	// }
	std::vector<std::shared_ptr<Vulkan::Pipeline>> pipelinesOld = { pipelineGPass, pipelineLPass, pipelineForward };
	Commands::Instance().OnRenderFinished([pipelinesOld]() {});
	CreateDeferredGPassPipeline();
	CreateDeferredLightPassPipeline();
	CreateForwardPipeline();
}
}