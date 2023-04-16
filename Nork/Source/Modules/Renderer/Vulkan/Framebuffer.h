#pragma once

#include "Image.h"
#include "RenderPass.h"

namespace Nork::Renderer::Vulkan {
    struct FramebufferCreateInfo : vk::FramebufferCreateInfo
    {
        FramebufferCreateInfo(const FramebufferCreateInfo& other)
            : vk::FramebufferCreateInfo::FramebufferCreateInfo(other),
            attachmentHandles(other.attachmentHandles), attachments(other.attachments)
        {
            setAttachments(attachmentHandles);
        }
        FramebufferCreateInfo(uint32_t width, uint32_t height, vk::RenderPass renderPass, 
            const std::vector<std::shared_ptr<ImageView>>& attachments, bool imageless = false)
            : attachmentHandles(ToHandles(attachments)), attachments(attachments)
        {
            this->renderPass = renderPass;
            this->flags = imageless ? vk::FramebufferCreateFlagBits::eImageless : (vk::FramebufferCreateFlagBits)0;
            this->width = width;
            this->height = height;
            this->layers = 1;
            setAttachments(attachmentHandles);
        }
    private:
        static std::vector<vk::ImageView> ToHandles(const std::vector<std::shared_ptr<ImageView>>& attachments)
        {
            std::vector<vk::ImageView> imgViews;
            imgViews.reserve(attachments.size());
            for (auto& att : attachments)
                imgViews.push_back(**att);
            return imgViews;
        }
    public:
        std::vector<vk::ImageView> attachmentHandles;
        std::vector<std::shared_ptr<ImageView>> attachments;
    };
    class Framebuffer: public vk::raii::Framebuffer
    {
    public:
        Framebuffer(const FramebufferCreateInfo& createInfo)
            : vk::raii::Framebuffer(Device::Instance(), createInfo), createInfo(createInfo)
        {
        }
        uint32_t Width() const { return createInfo.width; }
        uint32_t Height() const { return createInfo.height; }
        const std::vector<std::shared_ptr<ImageView>>& Attachments() const { return createInfo.attachments; }
    public:
        bool usedByCommandBuffer = false;
        FramebufferCreateInfo createInfo;
    };
}
