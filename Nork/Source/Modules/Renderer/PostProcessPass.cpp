#include "PostProcessPass.h"

#include "Vulkan/SwapChain.h"
#include "Data/Vertex.h"
#include "RendererSettings.h"

namespace Nork::Renderer {

enum class Stage : uint32_t {
	GaussianHorizontal = 0, GaussianVertical, Filter, Downsample, Upsample, Writeback, Tonemap
};
struct _PushConstant {
	Stage stage;
	uint32_t srcImgIdx;
};

static glm::uvec2 Resolution() {
	return *Settings::Instance().resolution;
}
static float Sigma() {
	return Settings::Instance().bloom->sigma;
}
static uint32_t KernelSize() {
	return Settings::Instance().bloom->gaussianKernelSize;
}
static uint32_t MipLevels() {
	return Settings::Instance().bloom->mipLevels;
}
static uint32_t MaxKernelSize() {
	return Settings::Instance().bloom->maxKernelSize;
}
static uint32_t MaxMipLevels() {
	return Settings::Instance().bloom->maxMipLevels();
}
static bool InlineKernelData() {
	return Settings::Instance().bloom->inlineKernelData;
}
static bool InlineKernelSize() {
	return Settings::Instance().bloom->inlineKernelSize;
}
static bool UseBlit() {
	return Settings::Instance().bloom->useBlitFromDownsampling;
}
static bool BlitLinear() {
	return Settings::Instance().bloom->blitLinear;
}
static glm::vec4 Threshold() {
	return Settings::Instance().bloom->threshold;
}
static bool InlineThreshold() {
	return Settings::Instance().bloom->inlineThreshold;
}
std::vector<float> CreateKernel() {
	if (KernelSize() % 2 == 0) {
		std::unreachable(); // should be dstributed equally around a pixel
	}
	std::vector<float> kernel(KernelSize());

	double s = 2.0 * Sigma() * Sigma();
	double sum = 0.0;
	int range = KernelSize() / 2;

	for (int i = -range; i <= range; i++) {
		double r = sqrt(i * i);
		kernel[i + range] = (exp(-(r * r) / s)) / (std::numbers::pi * s);
		sum += kernel[i + range];
	}

	for (auto& v : kernel) {
		v /= sum;
	}

	return kernel;
}
std::vector<std::array<std::string, 2>> GetMacros() {
	std::stringstream kernelSS;
	auto kernel = CreateKernel();
	kernelSS << kernel[0];
	for (size_t i = 1; i < kernel.size(); i++)
	{
		kernelSS << ", " << kernel[i];
	}
	std::stringstream thresholdSS;
	auto thr = Threshold();
	thr *= thr.a;
	thresholdSS << "vec3(" << thr.r << "," << thr.g << "," << thr.b << ")";

	return std::vector<std::array<std::string, 2>> {
		{ "GAUSSIAN_HORIZONTAL", std::to_string(std::to_underlying(Stage::GaussianHorizontal)) },
		{ "GAUSSIAN_VERTICAL", std::to_string(std::to_underlying(Stage::GaussianVertical)) },
		{ "FILTER", std::to_string(std::to_underlying(Stage::Filter)) },
		{ "DOWNSAMPLE", std::to_string(std::to_underlying(Stage::Downsample)) },
		{ "UPSAMPLE", std::to_string(std::to_underlying(Stage::Upsample)) },
		{ "WRITEBACK", std::to_string(std::to_underlying(Stage::Writeback)) },
		{ "TONEMAP", std::to_string(std::to_underlying(Stage::Tonemap)) },
		{ "RESOLUTION_X", std::to_string(Resolution().x) },
		{ "RESOLUTION_Y", std::to_string(Resolution().y) },
		{ "RESOLUTION", "vec2(RESOLUTION_X, RESOLUTION_Y)" },
		{ "KERNEL_ARRAY", kernelSS.str() },
		{ "KERNEL", InlineKernelData() ? "KERNEL_STATIC" : "KERNEL_DYNAMIC" },
		{ "MAX_KERNEL_SIZE", std::to_string(MaxKernelSize()) },
		{ "KERNEL_SIZE", InlineKernelSize() ? std::to_string(KernelSize()) : "KERNEL_SIZE_DYNAMIC" },
		{ "THRESHOLD", InlineThreshold() ? thresholdSS.str() : "THRESHOLD_DYNAMIC" },
		{ "EXPOSURE", Settings::Instance().postProcess->inlineExposure ? std::to_string(Settings::Instance().postProcess->exposure) : "EXPOSURE_DYNAMIC" },
	};
}

PostProcessPass::PostProcessPass(const std::shared_ptr<Image>& target)
	: target(target)
{
	Settings::Instance().postProcess.subscribe([&](const Settings::PostProcess& old, const Settings::PostProcess& val) {
		bool rebuildShader =
			old.inlineExposure != val.inlineExposure ||
			val.inlineExposure && (old.exposure != val.exposure)
			;
		if (rebuildShader) {
			RefreshShaders();
		}
	}, lifeCycle);
	Settings::Instance().bloom.subscribe([&](const Settings::Bloom& old, const Settings::Bloom& val) {
		bool rebuildShader =
			old.inlineKernelSize != val.inlineKernelSize || old.inlineKernelData != val.inlineKernelData || old.inlineThreshold != val.inlineThreshold ||
			val.inlineKernelSize && (old.gaussianKernelSize != val.gaussianKernelSize) ||
			val.inlineKernelData && (old.sigma != val.sigma) ||
			val.inlineThreshold && (old.threshold != val.threshold)
			;
		if (rebuildShader) {
			RefreshShaders();
		}
	}, lifeCycle);
	CreateTextures();
	CreatePipelineLayout();
	CreatePipeline();
	WriteDescriptorSets();
}
void PostProcessPass::CreateTextures()
{
	Vulkan::ImageCreateInfo imageCreateInfo(Resolution().x, Resolution().y, Vulkan::Format::rgba32f,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, MaxMipLevels());

	srcImage = std::make_shared<Image>(imageCreateInfo, vk::ImageAspectFlagBits::eColor, false, vk::PipelineStageFlags2{}, vk::AccessFlags2{}, false);
	pongImage = std::make_shared<Image>(Vulkan::ImageCreateInfo(Resolution().x, Resolution().y, Vulkan::Format::rgba32f, vk::ImageUsageFlagBits::eStorage), vk::ImageAspectFlagBits::eColor);

	auto setImageLayout = [&](vk::Image img, bool mip = true) {
		Commands::Instance().TransferCommand([&](Vulkan::CommandBuffer& cmd)
		{
			auto barrier = vk::ImageMemoryBarrier2()
				.setImage(img)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, mip ? MaxMipLevels() : 1, 0, 1))
				.setOldLayout(vk::ImageLayout::eUndefined)
				.setNewLayout(vk::ImageLayout::eGeneral)
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
				.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
				.setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite);
			cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
		});
	};
	setImageLayout(**srcImage->img);
	setImageLayout(**pongImage->img, false);

	for (size_t i = 0; i < MaxMipLevels(); i++)
	{
		auto view = std::make_shared<Vulkan::ImageView>(Vulkan::ImageViewCreateInfo(**srcImage->img, srcImage->img->Format(), vk::ImageAspectFlagBits::eColor, 1, i));
		srcMipmaps.push_back(view);
	}

	using namespace Vulkan;
}
void PostProcessPass::CreatePipelineLayout()
{
	descriptorSetLayout = std::make_shared<Vulkan::DescriptorSetLayout>(Vulkan::DescriptorSetLayoutCreateInfo()
		.Binding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute) // target
		.Binding(1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, MaxMipLevels())
		.Binding(2, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute));
	descriptorPool = std::make_shared<Vulkan::DescriptorPool>(
		Vulkan::DescriptorPoolCreateInfo({ descriptorSetLayout }, 1));
	descriptorSet = std::make_shared<Vulkan::DescriptorSet>(Vulkan::DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayout));

	vk::PushConstantRange push;
	push.size = sizeof(glm::vec4) * 2 + sizeof(uint32_t) + MaxKernelSize() * sizeof(float);
	push.stageFlags = vk::ShaderStageFlagBits::eCompute;
	vk::PushConstantRange pushKernel;
	pipelineLayout = std::make_shared<Vulkan::PipelineLayout>(
		Vulkan::PipelineLayoutCreateInfo({ **descriptorSetLayout }, { push }));
}
void PostProcessPass::CreatePipeline()
{
	Vulkan::ShaderModule shaderModule(LoadShader("Source/Shaders/postProcess.comp", GetMacros()), vk::ShaderStageFlagBits::eCompute);
	auto createInfo = vk::ComputePipelineCreateInfo()
		.setLayout(**pipelineLayout)
		.setStage(vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eCompute, *shaderModule, "main"));

	pipeline = std::make_shared<Vulkan::ComputePipeline>(createInfo);
}
void PostProcessPass::WriteDescriptorSets()
{
	auto textureSampler = std::make_shared<Vulkan::Sampler>();
	auto writer = descriptorSet->Writer()
		.Image(0, *target->view, vk::ImageLayout::eGeneral, *textureSampler, vk::DescriptorType::eStorageImage)
		.Image(2, *pongImage->view, vk::ImageLayout::eGeneral, *textureSampler, vk::DescriptorType::eStorageImage);
	for (size_t i = 0; i < MaxMipLevels(); i++)
	{
		writer.Image(1, *srcMipmaps[i], vk::ImageLayout::eGeneral, *textureSampler, vk::DescriptorType::eStorageImage, i);
	}
	writer.Write();
}
void PostProcessPass::RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame)
{
	cmd.bindPipeline(vk::PipelineBindPoint::eCompute, **pipeline);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, **pipelineLayout, 0, { **descriptorSet }, {});
	glm::vec2 groupSize(32, 32);
	auto w = target->img->Width();
	auto h = target->img->Height();

	_PushConstant pushConstant;

	auto dispatchStage = [&](int c = 0) {
		auto size = pushConstant.stage == Stage::Downsample ? groupSize / glm::vec2(2) : groupSize;
		cmd.pushConstants<_PushConstant>(**pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
		cmd.dispatch(std::ceil((w / (size.x * std::pow(2, pushConstant.srcImgIdx + c)))), (std::ceil(h / (size.y * std::pow(2, pushConstant.srcImgIdx + c)))), 1);
	};

	if (!Settings::Instance().bloom->inlineThreshold) {
		auto threshold = Threshold();
		threshold *= threshold.a;
		cmd.pushConstants<glm::vec4>(**pipelineLayout, vk::ShaderStageFlagBits::eCompute, sizeof(glm::vec4), threshold);
	}
	pushConstant.stage = Stage::Filter;
	pushConstant.srcImgIdx = 0;

	auto barrier = vk::ImageMemoryBarrier2()
		.setImage(**target->img).setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
		.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setNewLayout(vk::ImageLayout::eGeneral)
		.setSrcStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe).setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
		.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite).setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
	cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));

	if (!Settings::Instance().postProcess->inlineExposure) {
		cmd.pushConstants<float>(**pipelineLayout, vk::ShaderStageFlagBits::eCompute, sizeof(pushConstant), Settings::Instance().postProcess->exposure);
	}

	if (!Settings::Instance().postProcess->bloom) { // do only tonemapping

		pushConstant.stage = Stage::Tonemap;
		dispatchStage();

		barrier
			.setImage(**target->img)
			.setOldLayout(vk::ImageLayout::eGeneral).setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader).setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
			.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite).setDstAccessMask(vk::AccessFlagBits2::eShaderRead);
		cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));

		return;
	}

	dispatchStage();
	auto barrier11 = vk::ImageMemoryBarrier2()
		.setImage(**srcImage->img)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
		.setOldLayout(vk::ImageLayout::eGeneral)
		.setNewLayout(vk::ImageLayout::eGeneral)
		.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
		.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
		.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
		.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
	cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier11));

	if (!Settings::Instance().bloom->inlineKernelData) {
		cmd.pushConstants<uint32_t>(**pipelineLayout, vk::ShaderStageFlagBits::eCompute, 2 * sizeof(glm::vec4), KernelSize());
		cmd.pushConstants<float>(**pipelineLayout, vk::ShaderStageFlagBits::eCompute, 2 * sizeof(glm::vec4) + sizeof(uint32_t), CreateKernel());
	}

	size_t iter = MipLevels() - 1;
	for (size_t i = 0; i < iter; i++)
	{
		pushConstant.stage = Stage::GaussianHorizontal;
		dispatchStage();
		auto barrier4 = vk::ImageMemoryBarrier2()
			.setImage(**pongImage->img)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setOldLayout(vk::ImageLayout::eGeneral)
			.setNewLayout(vk::ImageLayout::eGeneral)
			.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
			.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
			.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
			.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
		cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier4));

		pushConstant.stage = Stage::GaussianVertical;
		dispatchStage();

		if (UseBlit()) {

			int32_t w = srcImage->img->Width() / std::pow(2, pushConstant.srcImgIdx);
			int32_t h = srcImage->img->Height() / std::pow(2, pushConstant.srcImgIdx);
			vk::ImageBlit blit;
			blit.srcOffsets[0] = vk::Offset3D(0);
			blit.srcOffsets[1] = vk::Offset3D(w, h, 1);
			blit.dstOffsets[0] = vk::Offset3D(0);
			blit.dstOffsets[1] = vk::Offset3D(w / 2, h / 2, 1); // vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1); // std::max(mipWidth / 2, 1), std::max(mipHeight / 2, 1), 1
			blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.srcSubresource.mipLevel = pushConstant.srcImgIdx;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.dstSubresource.mipLevel = pushConstant.srcImgIdx + 1;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			auto barrier4 = vk::ImageMemoryBarrier2()
				.setImage(**srcImage->img)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, pushConstant.srcImgIdx + 1, 1, 0, 1))
				.setOldLayout(vk::ImageLayout::eGeneral)
				.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
				.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setSrcAccessMask({})
				.setDstAccessMask(vk::AccessFlagBits2::eTransferWrite);
			auto barrier3 = vk::ImageMemoryBarrier2()
				.setImage(**srcImage->img)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, pushConstant.srcImgIdx, 1, 0, 1))
				.setOldLayout(vk::ImageLayout::eGeneral)
				.setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
				.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
				.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
				.setDstAccessMask(vk::AccessFlagBits2::eTransferRead);
			std::vector<vk::ImageMemoryBarrier2> barriers{ barrier3, barrier4 };
			cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barriers));

			cmd.blitImage(**srcImage->img, vk::ImageLayout::eTransferSrcOptimal, **srcImage->img, vk::ImageLayout::eTransferDstOptimal, blit,
				BlitLinear() ? vk::Filter::eLinear : vk::Filter::eNearest);

			auto barrier5 = vk::ImageMemoryBarrier2()
				.setImage(**srcImage->img)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, pushConstant.srcImgIdx + 1, 1, 0, 1))
				.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
				.setNewLayout(vk::ImageLayout::eGeneral)
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
				.setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
			auto barrier6 = vk::ImageMemoryBarrier2()
				.setImage(**srcImage->img)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, pushConstant.srcImgIdx, 1, 0, 1))
				.setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
				.setNewLayout(vk::ImageLayout::eGeneral)
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
				.setSrcAccessMask(vk::AccessFlagBits2::eTransferRead)
				.setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite);
			barriers = { barrier5 , barrier6 };
			cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barriers));

			pushConstant.srcImgIdx += 1;
		}
		else {
			auto barrier3 = vk::ImageMemoryBarrier2()
				.setImage(**srcImage->img)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, pushConstant.srcImgIdx, 1, 0, 1))
				.setOldLayout(vk::ImageLayout::eGeneral)
				.setNewLayout(vk::ImageLayout::eGeneral)
				.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
				.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
				.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
				.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
			cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier3));

			pushConstant.stage = Stage::Downsample;
			dispatchStage();
			pushConstant.srcImgIdx += 1;
			auto barrier2 = vk::ImageMemoryBarrier2()
				.setImage(**srcImage->img)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, pushConstant.srcImgIdx, 1, 0, 1))
				.setOldLayout(vk::ImageLayout::eGeneral)
				.setNewLayout(vk::ImageLayout::eGeneral)
				.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
				.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
				.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
				.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
			cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier2));
		}
	}
	for (size_t i = 0; i < iter; i++)
	{
		pushConstant.stage = Stage::Upsample;
		dispatchStage(-1);
		pushConstant.srcImgIdx--;

		auto barrier2 = vk::ImageMemoryBarrier2()
			.setImage(**srcImage->img)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, pushConstant.srcImgIdx, 1, 0, 1))
			.setOldLayout(vk::ImageLayout::eGeneral)
			.setNewLayout(vk::ImageLayout::eGeneral)
			.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
			.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
			.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
			.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
		cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier2));
	}
	pushConstant.stage = Stage::Writeback;
	dispatchStage();

	barrier
		.setImage(**target->img)
		.setOldLayout(vk::ImageLayout::eGeneral).setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader).setDstStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
		.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite).setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
	cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier));
}
void PostProcessPass::RefreshShaders()
{
	if (IsShaderSourceChanged("Source/Shaders/postProcess.comp", GetMacros())) {
		auto pipelineOld = pipeline;
		Commands::Instance().OnRenderFinished([pipelineOld]() {});
		CreatePipeline();
	}
}
}