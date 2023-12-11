#include "ColliderPass.h"
#include "Components/Physics.h"
#include "Components/Common.h"
#include "Modules/Renderer/Resources.h" // Resources.vp

namespace Nork {
ColliderPass::ColliderPass(const std::shared_ptr<Renderer::Image>& target)
	: enabled(false)
{
	instance = this;

	using MemoryFlags = Renderer::MemoryFlags;
	constexpr uint32_t maxVertexCount = 10000, maxIndexCount = 10000, framesInFlight = 2;
	vertexBuffer = std::make_shared<Renderer::DeviceArrays>(
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, maxVertexCount * sizeof(Vertex), framesInFlight,
		vk::PipelineStageFlagBits2::eVertexInput, vk::AccessFlagBits2::eVertexAttributeRead);
	indexBuffer = std::make_shared<Renderer::DeviceArrays>(
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, maxIndexCount * sizeof(uint32_t), framesInFlight,
		vk::PipelineStageFlagBits2::eIndexInput, vk::AccessFlagBits2::eIndexRead);
	vertices = vertexBuffer->New<Vertex>(maxVertexCount);
	indices = indexBuffer->New<uint32_t>(maxIndexCount);

	CreateRenderPass();

	auto w = target->img->Width();
	auto h = target->img->Height();
	using enum vk::ImageUsageFlagBits;
	depth = std::make_shared<Renderer::Image>(w, h, Vulkan::Format::depth32, eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth);
	color = target;
	fb = std::make_shared<Vulkan::Framebuffer>(Vulkan::FramebufferCreateInfo(w, h, **renderPass, { color->view, depth->view }));

	CreateGraphicsPipeline();
}
void ColliderPass::CreateGraphicsPipeline()
{
	using namespace Vulkan;
	ShaderModule vertShaderModule(LoadShader("Source/Shaders/position.vert"), vk::ShaderStageFlagBits::eVertex);
	ShaderModule fragShaderModule(LoadShader("Source/Shaders/line.frag"), vk::ShaderStageFlagBits::eFragment);

	vk::PushConstantRange vpPush;
	vpPush.size = sizeof(glm::mat4);
	vpPush.stageFlags = vk::ShaderStageFlagBits::eVertex;
	vk::PushConstantRange colorPush;
	colorPush.offset = vpPush.size;
	colorPush.size = sizeof(glm::vec3);
	colorPush.stageFlags = vk::ShaderStageFlagBits::eFragment;
	pipelineLayout = std::make_shared<PipelineLayout>(PipelineLayoutCreateInfo({}, { vpPush, colorPush }));

	pipeline = std::make_shared<Vulkan::Pipeline>(Vulkan::PipelineCreateInfo()
		.Layout(**pipelineLayout)
		.AddShader(vertShaderModule)
		.AddShader(fragShaderModule)
		.VertexInput<Vertex>()
		.InputAssembly(vk::PrimitiveTopology::eLineList)
		.Rasterization(false)
		.Multisampling()
		.ColorBlend(1)
		.RenderPass(**renderPass, 0)
		.DepthStencil(false, true, vk::CompareOp::eLess)
		.AdditionalDynamicStates({ vk::DynamicState::eLineWidth })
	);
}
void ColliderPass::CreateRenderPass()
{
	using namespace Vulkan;
	Vulkan::RenderPassCreateInfo createInfo;

	uint32_t colAttIdx = 0, depthAttIdx = 1;
	createInfo.Attachments(std::vector<AttachmentDescription>(2));
	createInfo.attachments[colAttIdx]
		.setInitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setFormat(Format::rgba32f)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eLoad)
		.setStoreOp(vk::AttachmentStoreOp::eStore);
	createInfo.attachments[depthAttIdx]
		.setFormat(Format::depth32)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear);

	std::vector<SubpassDescription> subPasses(1);
	subPasses[0]
		.ColorAttachments({
			{ colAttIdx, vk::ImageLayout::eColorAttachmentOptimal },
			})
			.DepthAttachment(depthAttIdx);
	createInfo.Subpasses(subPasses);

	createInfo.Dependencies({
		vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		});
	renderPass = std::make_shared<Vulkan::RenderPass>(createInfo);
}
void ColliderPass::RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame)
{
	if (!enabled)
		return;
	using namespace Vulkan;
	BeginRenderPass(**renderPass, *fb, cmd);
	cmd.setViewport(0, ViewportFull(*fb));
	cmd.setScissor(0, ScissorFull(*fb));

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);
	cmd.pushConstants<glm::mat4>(**pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, Renderer::Resources::Instance().vp);
	cmd.pushConstants<glm::vec3>(**pipelineLayout, vk::ShaderStageFlagBits::eFragment, sizeof(glm::mat4), lineColor);

	cmd.bindVertexBuffers(0, **vertexBuffer->buffer->Underlying(), { 0 });
	cmd.bindIndexBuffer(**indexBuffer->buffer->Underlying(), 0, vk::IndexType::eUint32);
	cmd.setLineWidth(2);
	cmd.drawIndexed(drawIndexCount, 1, 0, 0, 0);

	cmd.endRenderPass();
}

void ColliderPass::UpdateBuffers(const entt::registry& registry) {
	static std::vector<ColliderPass::Vertex> vertices;
	static std::vector<uint32_t> indices;
	if (!enabled) {
		vertices.resize(0);
		indices.resize(0);
		return;
	}

	vertices.clear(); indices.clear();
	uint32_t vertexOffset = 0;
	for (auto [ent, tr, phx] : registry.view<Components::Transform, Components::Physics>().each()) {
		for (auto& collider : phx.Colliders()) {
			vertexOffset = vertices.size();
			if (useGlobalColliderVertices) {
				for (auto& vert : collider.global.verts) {
					vertices.push_back(ColliderPass::Vertex{ .position = vert });
				}
			}
			else {
				for (auto& vert : collider.local.verts) {
					vertices.push_back(ColliderPass::Vertex{ .position = tr.modelMatrix * glm::vec4(vert + collider.offset, 1.0f) });
				}
			}
			for (auto& edge : collider.local.edges) {
				indices.push_back(vertexOffset + edge.first);
			 	indices.push_back(vertexOffset + edge.second);
			}
		}
		// indices.push_back(vertices.size());
		// indices.push_back(vertices.size() + 1);
		// vertices.push_back({ phx.Object().aabb.min });
		// vertices.push_back({ phx.Object().aabb.max });
	}
	drawIndexCount = indices.size();
	this->vertices->Write(vertices.data(), vertices.size());
	this->indices->Write(indices.data(), indices.size());
}

ColliderPass* ColliderPass::instance = nullptr;
}