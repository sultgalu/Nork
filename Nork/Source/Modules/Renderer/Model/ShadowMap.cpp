#include "ShadowMap.h"
#include "../ShadowMapPass.h"
#include "../Resources.h"

namespace Nork::Renderer {

void FreeFramebuffer(std::shared_ptr<Vulkan::Framebuffer>& fb, int imgIdx) {
	if (fb->usedByCommandBuffer) {
		auto fbInFlight = fb;
		Commands::Instance().OnRenderFinished([fbInFlight]() {
		});
	}
	Resources::Instance().dirShadowDescriptors->RemoveImage(imgIdx, fb->usedByCommandBuffer);
	fb = nullptr;
}
DirShadowMap::DirShadowMap()
{
	fb = nullptr;
}
DirShadowMap::~DirShadowMap()
{
	FreeFramebuffer(fb, Shadow()->shadMap);
}
void DirShadowMap::CreateTexture(uint32_t width, uint32_t height)
{
	if (fb) {
		FreeFramebuffer(fb, Shadow()->shadMap);
	}

	image = std::make_shared<Image>(width, height, ShadowMapPass::Instance().Format(),
		vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::ImageAspectFlagBits::eDepth);
	image->sampler = ShadowMapPass::Instance().sampler;

	fb = std::make_shared<Vulkan::Framebuffer>(Vulkan::FramebufferCreateInfo(
		width, height, **ShadowMapPass::Instance().renderPass, { image->view }));

	uint32_t descriptorIdx = Resources::Instance().dirShadowDescriptors->AddImage(image);
	Shadow()->shadMap = descriptorIdx;
}
PointShadowMap::PointShadowMap()
{
	fb = nullptr;
}
PointShadowMap::~PointShadowMap()
{
	FreeFramebuffer(fb, Shadow()->shadMap);
}
void PointShadowMap::CreateTexture(uint32_t size)
{
	if (fb) {
		FreeFramebuffer(fb, Shadow()->shadMap);
	}

	Vulkan::ImageCreateInfo createInfo(size, size, ShadowMapPass::Instance().Format(),
		vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
	createInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible)
		.setArrayLayers(6);
	image = std::make_shared<Image>(createInfo, vk::ImageAspectFlagBits::eDepth, true);
	image->sampler = ShadowMapPass::Instance().samplerCube;

	fb = std::make_shared<Vulkan::Framebuffer>(Vulkan::FramebufferCreateInfo(
		size, size, **ShadowMapPass::Instance().renderPass, { image->view }));

	uint32_t descriptorIdx = Resources::Instance().pointShadowDescriptors->AddImage(image);
	Shadow()->shadMap = descriptorIdx;
}
}