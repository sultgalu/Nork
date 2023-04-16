#pragma once

#include "Modules/Renderer/RenderPass.h"
#include "Modules/Renderer/Vulkan/SwapChain.h"
#include "Modules/Renderer/Image.h"

namespace Nork::Editor {
class EditorPass : public Nork::Renderer::RenderPass
{
public:
	EditorPass();
	void OnFramebufferResized() override;
	void CreateFramebuffer();
	~EditorPass();
	void CreateRenderPassUI();
	void InitImguiForVulkan();
	void OnTransferCommands() override;
	void RecordCommandBuffer(Nork::Renderer::Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame) override;
public:
	std::shared_ptr<Nork::Renderer::Image> imgUI;
	std::shared_ptr<Nork::Renderer::Vulkan::Framebuffer> fbUI;
	std::shared_ptr<Nork::Renderer::Vulkan::DescriptorPool> imguiPool;
	std::shared_ptr<Nork::Renderer::Vulkan::RenderPass> renderPassUI;
};
}