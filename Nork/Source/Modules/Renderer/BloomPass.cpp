#include "BloomPass.h"

#include "Vulkan/SwapChain.h"
#include "Data/Vertex.h"
#include "RendererSettings.h"

namespace Nork::Renderer {

enum class Stage : uint32_t {
	GaussianHorizontal = 0, GaussianVertical, Filter, Downsample, Upsample, Writeback
};
struct _PushConstant {
	Stage stage;
	uint32_t srcImgIdx;
};

static LifeCycle lifeCycle;

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

	return std::vector<std::array<std::string, 2>> {
		{ "GAUSSIAN_HORIZONTAL", std::to_string(std::to_underlying(Stage::GaussianHorizontal)) },
		{ "GAUSSIAN_VERTICAL", std::to_string(std::to_underlying(Stage::GaussianVertical)) },
		{ "FILTER", std::to_string(std::to_underlying(Stage::Filter)) },
		{ "DOWNSAMPLE", std::to_string(std::to_underlying(Stage::Downsample)) },
		{ "UPSAMPLE", std::to_string(std::to_underlying(Stage::Upsample)) },
		{ "WRITEBACK", std::to_string(std::to_underlying(Stage::Writeback)) },
		{ "RESOLUTION_X", std::to_string(Resolution().x) },
		{ "RESOLUTION_Y", std::to_string(Resolution().y) },
		{ "RESOLUTION", "vec2(RESOLUTION_X, RESOLUTION_Y)" },
		{ "KERNEL_ARRAY", kernelSS.str() },
		{ "KERNEL", InlineKernelData() ? "KERNEL_STATIC" : "KERNEL_DYNAMIC" },
		{ "MAX_KERNEL_SIZE", std::to_string(MaxKernelSize()) },
		{ "KERNEL_SIZE", InlineKernelSize() ? std::to_string(KernelSize()) : "KERNEL_SIZE_DYNAMIC" },
	};
}

BloomPass::BloomPass(const std::shared_ptr<Image>& target)
	: target(target)
{
	Settings::Instance().bloom.subscribe([&](const Settings::Bloom& old, const Settings::Bloom& val) {
		bool rebuildShader =
			old.inlineKernelSize != val.inlineKernelSize || old.inlineKernelData != val.inlineKernelData ||
			val.inlineKernelSize && (old.gaussianKernelSize != val.gaussianKernelSize) ||
			val.inlineKernelData && (old.sigma != val.sigma);
		if (rebuildShader) {
			RefreshShaders();
		}
	}, lifeCycle);
	CreateTextures();
	CreatePipelineLayout();
	CreatePipeline();
	WriteDescriptorSets();
}
void BloomPass::CreateTextures()
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
void BloomPass::CreatePipelineLayout()
{
	descriptorSetLayout = std::make_shared<Vulkan::DescriptorSetLayout>(Vulkan::DescriptorSetLayoutCreateInfo()
		.Binding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute) // target
		.Binding(1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, MaxMipLevels())
		.Binding(2, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute));
	descriptorPool = std::make_shared<Vulkan::DescriptorPool>(
		Vulkan::DescriptorPoolCreateInfo({ descriptorSetLayout }, 1));
	descriptorSet = std::make_shared<Vulkan::DescriptorSet>(Vulkan::DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayout));

	vk::PushConstantRange push;
	push.size = sizeof(_PushConstant) + sizeof(uint32_t) + MaxKernelSize() * sizeof(float);
	push.stageFlags = vk::ShaderStageFlagBits::eCompute;
	vk::PushConstantRange pushKernel;
	pipelineLayout = std::make_shared<Vulkan::PipelineLayout>(
		Vulkan::PipelineLayoutCreateInfo({ **descriptorSetLayout }, { push }));
}
void BloomPass::CreatePipeline()
{
	Vulkan::ShaderModule shaderModule(LoadShader("Source/Shaders/bloom.comp", GetMacros()), vk::ShaderStageFlagBits::eCompute);
	auto createInfo = vk::ComputePipelineCreateInfo()
		.setLayout(**pipelineLayout)
		.setStage(vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eCompute, *shaderModule, "main"));

	pipeline = std::make_shared<Vulkan::ComputePipeline>(createInfo);
}
void BloomPass::WriteDescriptorSets()
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
void BloomPass::RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame)
{
	auto barrier = vk::ImageMemoryBarrier2()
		.setImage(**target->img)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
		.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setNewLayout(vk::ImageLayout::eGeneral)
		.setSrcStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)
		.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
		.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
		.setDstAccessMask(vk::AccessFlagBits2::eShaderStorageRead);
	cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier).setDependencyFlags(vk::DependencyFlagBits::eByRegion));

	cmd.bindPipeline(vk::PipelineBindPoint::eCompute, **pipeline);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, **pipelineLayout, 0, { **descriptorSet }, {});
	if (!Settings::Instance().bloom->inlineKernelData) {
		cmd.pushConstants<uint32_t>(**pipelineLayout, vk::ShaderStageFlagBits::eCompute, sizeof(_PushConstant), KernelSize());
		cmd.pushConstants<float>(**pipelineLayout, vk::ShaderStageFlagBits::eCompute, sizeof(_PushConstant) + sizeof(uint32_t), CreateKernel());
	}

	glm::vec2 groupSize(32, 32);
	auto w = target->img->Width();
	auto h = target->img->Height();

	_PushConstant pushConstant;

	auto dispatchStage = [&](int c = 0) {
		auto size = pushConstant.stage == Stage::Downsample ? groupSize / glm::vec2(2) : groupSize;
		cmd.pushConstants<_PushConstant>(**pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
		cmd.dispatch(std::ceil((w / (size.x * std::pow(2, pushConstant.srcImgIdx + c)))), (std::ceil(h / (size.y * std::pow(2, pushConstant.srcImgIdx + c)))), 1);
	};

	pushConstant.stage = Stage::Filter;
	pushConstant.srcImgIdx = 0;
	dispatchStage();

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
		cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier4).setDependencyFlags(vk::DependencyFlagBits::eByRegion));
		pushConstant.stage = Stage::GaussianVertical;
		dispatchStage();
		auto barrier3 = vk::ImageMemoryBarrier2()
			.setImage(**srcImage->img)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, pushConstant.srcImgIdx, 1, 0, 1))
			.setOldLayout(vk::ImageLayout::eGeneral)
			.setNewLayout(vk::ImageLayout::eGeneral)
			.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
			.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
			.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
			.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
		cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier3).setDependencyFlags(vk::DependencyFlagBits::eByRegion));

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
			std::vector<vk::ImageMemoryBarrier2> barriers { barrier3, barrier4 };
			cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barriers).setDependencyFlags(vk::DependencyFlagBits::eByRegion));

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
			cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barriers).setDependencyFlags(vk::DependencyFlagBits::eByRegion));

			pushConstant.srcImgIdx += 1;
		} 
		else {
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
			cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier2).setDependencyFlags(vk::DependencyFlagBits::eByRegion));
		}
	}
	for (size_t i = 0; i < iter; i++)
	{
		pushConstant.stage = Stage::Upsample;
		dispatchStage(-1);
		pushConstant.srcImgIdx -= 1;

		auto barrier2 = vk::ImageMemoryBarrier2()
			.setImage(**srcImage->img)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, pushConstant.srcImgIdx, 1, 0, 1))
			.setOldLayout(vk::ImageLayout::eGeneral)
			.setNewLayout(vk::ImageLayout::eGeneral)
			.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
			.setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
			.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
			.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
		cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier2).setDependencyFlags(vk::DependencyFlagBits::eByRegion));
	}
	pushConstant.stage = Stage::Writeback;
	dispatchStage();

	barrier
		.setImage(**target->img)
		.setOldLayout(vk::ImageLayout::eGeneral)
		.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
		.setDstStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
		.setSrcAccessMask(vk::AccessFlagBits2::eShaderStorageWrite)
		.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead);
	cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrier).setDependencyFlags(vk::DependencyFlagBits::eByRegion));
}
void BloomPass::RefreshShaders()
{
	if (IsShaderSourceChanged("Source/Shaders/bloom.comp", GetMacros())) {
		auto pipelineOld = pipeline;
		Commands::Instance().OnRenderFinished([pipelineOld]() {});
		CreatePipeline();
	}
}
}