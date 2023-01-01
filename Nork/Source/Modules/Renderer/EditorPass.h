#pragma once

#include "RenderPass.h"
#include "Vulkan/SwapChain.h"
#include "Image.h"

namespace Nork::Renderer {
class EditorPass : public RenderPass
{
public:
	EditorPass()
	{
		createRenderPassUI();
		auto w = Vulkan::SwapChain::Instance().Width();
		auto h = Vulkan::SwapChain::Instance().Height();
		imgUI = std::make_shared<Image>(w, h, Vulkan::Format::rgba8Unorm,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
			vk::ImageAspectFlagBits::eColor);
		fbUI = std::make_shared<Vulkan::Framebuffer>(Vulkan::FramebufferCreateInfo(w, h, **renderPassUI, { imgUI->view }));
		InitImguiForVulkan();
	}
	~EditorPass()
	{
		ImGui_ImplVulkan_Shutdown();
	}
	void createRenderPassUI()
	{
		using namespace Vulkan;
		RenderPassCreateInfo createInfo;

		uint32_t colAtt = 0;
		createInfo.Attachments(std::vector<AttachmentDescription>(1));
		createInfo.attachments[colAtt]
			.setFormat(Format::rgba8Unorm)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setFinalLayout(vk::ImageLayout::eTransferSrcOptimal);

		uint32_t sPass = 0;
		createInfo.Subpasses({
			SubpassDescription().ColorAttachments({ { colAtt, vk::ImageLayout::eColorAttachmentOptimal } })
			});

		createInfo.Dependencies({
			vk::SubpassDependency(sPass, VK_SUBPASS_EXTERNAL)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			.setDstStageMask(vk::PipelineStageFlagBits::eTransfer)
			.setDstAccessMask(vk::AccessFlagBits::eTransferRead),
			});
		renderPassUI = std::make_shared<Vulkan::RenderPass>(createInfo);
	}
	void InitImguiForVulkan()
	{
		using namespace Vulkan;
		std::vector<vk::DescriptorPoolSize> pool_sizes =
		{
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer , 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 }
		};

		imguiPool = std::make_shared<DescriptorPool>(DescriptorPoolCreateInfo(pool_sizes, 1000));

		//this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = *Instance::StaticInstance();
		init_info.PhysicalDevice = *PhysicalDevice::Instance();
		init_info.Device = *Device::Instance();
		init_info.Queue = *Device::Instance().graphicsQueue;
		init_info.DescriptorPool = **imguiPool;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Subpass = 0;

		ImGui_ImplVulkan_Init(&init_info, **renderPassUI);

		//execute a gpu command to upload imgui font textures
		Commands::Instance().TransferCommand([&](CommandBuffer& cmd)
		{
			ImGui_ImplVulkan_CreateFontsTexture(*cmd);
		});
		Commands::Instance().OnTransfersFinished([]()
		{ //clear font textures from cpu data
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		});
	}
	void recordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame) override
	{
		using namespace Vulkan;
		BeginRenderPass(**renderPassUI, *fbUI, cmd);
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);
		cmd.endRenderPass();

		auto& uiImg = *fbUI->Attachments()[0]->Image();
		auto barrierSc = vk::ImageMemoryBarrier2()
			.setImage(SwapChain::Instance().images[imageIndex])
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))

			.setOldLayout(vk::ImageLayout::eUndefined)
			.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
			.setSrcAccessMask(vk::AccessFlagBits2::eNone)

			.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
			.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
			.setDstAccessMask(vk::AccessFlagBits2::eTransferWrite);

		cmd.pipelineBarrier2(vk::DependencyInfo(vk::DependencyFlagBits::eByRegion)
			.setImageMemoryBarriers(barrierSc));

		cmd.copyImage(*uiImg, vk::ImageLayout::eTransferSrcOptimal,
			SwapChain::Instance().images[imageIndex], vk::ImageLayout::eTransferDstOptimal,
			vk::ImageCopy(
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {},
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {},
				{ SwapChain::Instance().Width(), SwapChain::Instance().Height(), 1 }
		));

		barrierSc
			.setOldLayout(barrierSc.newLayout)
			.setSrcStageMask(barrierSc.dstStageMask)
			.setSrcAccessMask(barrierSc.dstAccessMask)

			.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
			.setDstStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)
			.setDstAccessMask(vk::AccessFlagBits2::eNone);
		cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrierSc));
	}
public:
	std::shared_ptr<Image> imgUI;
	std::shared_ptr<Vulkan::Framebuffer> fbUI;
	std::shared_ptr<Vulkan::DescriptorPool> imguiPool;
	std::shared_ptr<Vulkan::RenderPass> renderPassUI;
};
}