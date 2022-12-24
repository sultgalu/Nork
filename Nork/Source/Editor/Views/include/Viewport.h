#pragma once

#include "../../View.h"
#include "Modules/Renderer/Vulkan/Image.h"

namespace Nork::Editor {
	class ViewportView: public View
	{
	public:
		ViewportView();
		void Content() override;
		struct MouseState
		{
			int mousePosX = 0, mousePosY = 0;
			bool isViewportHovered = false;
			bool isViewportDoubleClicked = false;
			bool isViewportClicked = false;
		};
		void SetImage(std::shared_ptr<Renderer::Vulkan::ImageView> image, std::shared_ptr<Renderer::Vulkan::Sampler> sampler)
		{
			// if (viewportImgDs != VK_NULL_HANDLE)
				// destroy?
			this->image = image;
			this->sampler = sampler;
			viewportImgDs = ImGui_ImplVulkan_AddTexture(**sampler, **image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	public:
		std::shared_ptr<Renderer::Vulkan::ImageView> image;
		std::shared_ptr<Renderer::Vulkan::Sampler> sampler;
		VkDescriptorSet viewportImgDs = VK_NULL_HANDLE;
		MouseState mouseState;
		// std::shared_ptr<SceneView> sceneView;
		std::shared_ptr<CameraController> camController = std::make_shared<FpsCameraController>();
	};
}