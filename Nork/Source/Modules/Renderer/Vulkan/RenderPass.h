#pragma once

#include "Device.h"

namespace Nork::Renderer::Vulkan {
    struct SubpassDescription
    {
    public:
        using Self = SubpassDescription;
        Self& InputAttachments(const std::vector<vk::AttachmentReference>& atts)
        {
            inputAttachments = atts;
            return *this;
        }
        Self& ColorAttachments(const std::vector<vk::AttachmentReference>& atts)
        {
            colorAttachments = atts;
            return *this;
        }
        Self& DepthAttachment(uint32_t idx, vk::ImageLayout layout = vk::ImageLayout::eDepthStencilAttachmentOptimal)
        {
            depthAttachment.emplace(vk::AttachmentReference(idx, layout));
            return *this;
        }
        std::vector<vk::AttachmentReference> colorAttachments;
        std::vector<vk::AttachmentReference> inputAttachments;
        std::optional<vk::AttachmentReference> depthAttachment;
    };

    struct AttachmentDescription : vk::AttachmentDescription
    {
        AttachmentDescription()
        {
            this->format = format;
            this->flags = {};
            this->samples = vk::SampleCountFlagBits::e1;
            this->loadOp = vk::AttachmentLoadOp::eDontCare;
            this->storeOp = vk::AttachmentStoreOp::eDontCare;
            this->stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            this->stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            this->initialLayout = vk::ImageLayout::eUndefined;
            this->finalLayout = vk::ImageLayout::eUndefined;
        }
    };

    struct RenderPassCreateInfo : vk::RenderPassCreateInfo
    {
    public:
        RenderPassCreateInfo() = default;
        RenderPassCreateInfo(RenderPassCreateInfo&&) = default;
        void Dependencies(const std::vector<vk::SubpassDependency>& dependencies)
        {
            this->dependencies = dependencies;
            setDependencies(this->dependencies);
        }
        void Attachments(const std::vector<AttachmentDescription>& attachments)
        {
            this->attachments = attachments;
            setAttachments(this->attachments);
        }
        void Subpasses(const std::vector<SubpassDescription>& subPasses)
        {
            subPassesHolder = subPasses;
            for (auto& sp : subPassesHolder)
                this->subPasses.push_back(vk::SubpassDescription()
                .setColorAttachments(sp.colorAttachments)
                .setInputAttachments(sp.inputAttachments)
                .setPDepthStencilAttachment(sp.depthAttachment.has_value() ? &sp.depthAttachment.value() : nullptr)
                );
            setSubpasses(this->subPasses);
        }
    public:
        std::vector<SubpassDescription> subPassesHolder;
        std::vector<vk::SubpassDescription> subPasses;
        std::vector<AttachmentDescription> attachments;
        std::vector<vk::SubpassDependency> dependencies;
    };

    class RenderPass: public vk::raii::RenderPass
    {
    public:
        RenderPass(RenderPassCreateInfo& createInfo)
            : vk::raii::RenderPass(Device::Instance(), createInfo),
            createInfo(std::move(createInfo))
        {}
    public:
        const RenderPassCreateInfo createInfo;
    };
}
