#pragma once

#include "Image.h"
#include "RenderPass.h"

namespace Nork::Renderer::Vulkan {
    class Framebuffer
    {
    public:
        // using Base = vk::raii::Framebuffer;

        Framebuffer(const Framebuffer&) = delete;
        static std::vector<VkImageView> ToImageViews(const std::vector<std::shared_ptr<ImageView>>& attachments)
        {
            std::vector<VkImageView> imgViews;
            imgViews.reserve(attachments.size());
            for (auto& att : attachments)
                imgViews.push_back(**att);
            return imgViews;
        }
        Framebuffer(uint32_t width, uint32_t height, const RenderPass& renderPass, const std::vector<std::shared_ptr<ImageView>>& attachments)
            : Framebuffer(width, height, renderPass, ToImageViews(attachments))
        {
            this->attachments = attachments;
        }
        ~Framebuffer()
        {
            vkDestroyFramebuffer(*Device::Instance(), handle, nullptr);
        }
    private:
        Framebuffer(uint32_t width, uint32_t height, const RenderPass& renderPass, const std::vector<VkImageView>& attachments)
            : width(width), height(height), renderPassConfig(renderPass.config)
        {
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass.handle;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;

            vkCreateFramebuffer(*Device::Instance(), &framebufferInfo, nullptr, &handle) == VkSuccess();
        }
    public:
        VkFramebuffer handle;
        std::vector<std::shared_ptr<ImageView>> attachments;
        const RenderPass::Config& renderPassConfig; // compatibility
        const uint32_t width, height;
    };
}
