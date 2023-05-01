#pragma once

#include "Vulkan/Pipeline.h"
#include "Vulkan/Framebuffer.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Shaderc.h"
#include "Utils/LifeCycle.h"

namespace Nork::Renderer {
class RenderPass
{
public:
	virtual void RefreshShaders() {}
	bool IsShaderSourceChanged(const fs::path& srcPath, std::vector<std::array<std::string, 2>> macros);
	std::vector<uint32_t> LoadShader(const fs::path& srcPath, std::vector<std::array<std::string, 2>> macros = {});
	void BeginRenderPass(vk::RenderPass renderPass, Vulkan::Framebuffer& fb, Vulkan::CommandBuffer& cmd);
	vk::Viewport ViewportFull(const Vulkan::Framebuffer& fb)
	{
		return vk::Viewport(0, 0, fb.Width(), fb.Height(), 0, 1);
	}
	vk::Rect2D ScissorFull(const Vulkan::Framebuffer& fb)
	{
		return vk::Rect2D({ 0, 0 }, { fb.Width(), fb.Height() });
	}
	virtual void OnTransferCommands() {}
	virtual void OnFramebufferResized() {}
	virtual void RecordCommandBuffer(Vulkan::CommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t currentFrame) = 0;
public:
protected:
	LifeCycle lifeCycle;
};
}