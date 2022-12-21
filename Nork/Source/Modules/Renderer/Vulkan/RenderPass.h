#pragma once

#include "Device.h"

namespace Nork::Renderer::Vulkan {
    struct SubPass
    {
    public:
        SubPass(uint32_t subPassIdx = -2)
            : subPassIdx(subPassIdx)
        {}
        using Self = SubPass;
        Self& InputAttachment(uint32_t idx, VkImageLayout layout)
        {
            inputAttachmentRefs.push_back(VkAttachmentReference{ .attachment = idx, .layout = layout });
            return *this;
        }
        Self& ColorAttachment(uint32_t idx, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            colorAttachmentRefs.push_back(VkAttachmentReference{ .attachment = idx, .layout = layout });
            return *this;
        }
        Self& DepthAttachment(uint32_t idx, VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            depthAttachmentRef.emplace(VkAttachmentReference{ .attachment = idx, .layout = layout });
            return *this;
        }
        VkSubpassDescription BuildDescription() const
        {
            VkSubpassDescription description{};
            description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            description.colorAttachmentCount = colorAttachmentRefs.size();
            description.pColorAttachments = colorAttachmentRefs.data();
            description.inputAttachmentCount = inputAttachmentRefs.size();
            description.pInputAttachments = inputAttachmentRefs.data();
            if (depthAttachmentRef.has_value())
                description.pDepthStencilAttachment = &depthAttachmentRef.value();
            return description;
        }
        std::vector<VkAttachmentReference> colorAttachmentRefs;
        std::optional<VkAttachmentReference> depthAttachmentRef;
        std::vector<VkAttachmentReference> inputAttachmentRefs;
        uint32_t subPassIdx;
    };

    struct AttachmentDescription : VkAttachmentDescription
    {
        static AttachmentDescription Depth(VkFormat format = VK_FORMAT_D32_SFLOAT)
        {
            return AttachmentDescription(format)
                .FinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        }
        static AttachmentDescription CleanDepth(VkFormat format = VK_FORMAT_D32_SFLOAT)
        {
            return AttachmentDescription(format)
                .LoadOp_Clear()
                .FinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        }
        static AttachmentDescription SwapChainImage(VkFormat format = VK_FORMAT_R8G8B8A8_SRGB)
        {
            return AttachmentDescription(format)
                .LoadOp_Clear()
                .StoreOp_Store()
                .FinalLayout_Present();
        }
        static AttachmentDescription ColorInternalUse(VkFormat format = VK_FORMAT_R8G8B8A8_SRGB)
        {
            return AttachmentDescription(format)
                .LoadOp_Clear();
        }
        static AttachmentDescription ColorForLaterCopy(VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool clear = true)
        {
            auto att = AttachmentDescription(format)
                // .LoadOp_Clear()
                .StoreOp_Store()
                .FinalLayout_TransferSrc();
            return clear ? att.LoadOp_Clear() : att;
        }
        static AttachmentDescription ColorForShaderRead(VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool clear = true)
        {
            auto att = AttachmentDescription(format)
                .StoreOp_Store()
                .FinalLayout_ShaderRead();
            return clear ? att.LoadOp_Clear() : att;
        }

        AttachmentDescription(VkFormat format = VK_FORMAT_UNDEFINED)
        {
            this->format = format;
            this->flags = 0;
            this->samples = VK_SAMPLE_COUNT_1_BIT;
            this->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            this->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            this->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            this->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            this->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            this->finalLayout = VK_IMAGE_LAYOUT_UNDEFINED; // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> copy target 
        }
        using Self = AttachmentDescription;
        Self& Samples(VkSampleCountFlagBits val) { this->samples = val; return *this; }
        Self& LoadOp(VkAttachmentLoadOp val) { this->loadOp = val; return *this; }
        Self& StoreOp(VkAttachmentStoreOp val) { this->storeOp = val; return *this; }
        Self& InitialLayout(VkImageLayout val) { this->initialLayout = val; return *this; }
        Self& FinalLayout(VkImageLayout val) { this->finalLayout = val; return *this; }

        Self& StoreOp_Store() { return StoreOp(VK_ATTACHMENT_STORE_OP_STORE); }
        Self& LoadOp_Clear() { return LoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR); }
        Self& FinalLayout_Present() { return FinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); }
        Self& FinalLayout_TransferSrc() { return FinalLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL); }
        Self& FinalLayout_ShaderRead() { return FinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); }
    };

    struct SubPassDependency : VkSubpassDependency
    {
        using Self = SubPassDependency;
        Self& SrcAccessMask(VkAccessFlags flags) { this->dstAccessMask = flags; return *this; }
        Self& DstAccessMask(VkAccessFlags flags) { this->dstAccessMask = flags; return *this; }
        Self& SrcStageMask(VkPipelineStageFlags flags) { this->srcStageMask = flags; return *this; }
        Self& DstStageMask(VkPipelineStageFlags flags) { this->dstStageMask = flags; return *this; }
        Self& Flags(VkDependencyFlags flags) { this->dependencyFlags = flags; return *this; }
        Self& Flags_ByRegion() { return Flags(VK_DEPENDENCY_BY_REGION_BIT); }
    };

    class RenderPass
    {
    public:
        class Config
        {
        public:
            Config(uint32_t attachmentCount, uint32_t subPassCount)
                : attachments(attachmentCount), subPasses(subPassCount)
            {}
            using Self = Config;
            Self& Dependency(uint32_t src, uint32_t dst, SubPassDependency dependency)
            {
                dependency.srcSubpass = src;
                dependency.dstSubpass = dst;
                dependencies.push_back(dependency);
                return *this;
            }
            Self& Dependency(const SubPass& src, const SubPass& dst, SubPassDependency dependency)
            {
                return Dependency(src.subPassIdx, dst.subPassIdx, dependency);
            }
            Self& DependencyExternalSrc(const SubPass& dst, SubPassDependency dependency)
            {
                return Dependency(VK_SUBPASS_EXTERNAL, dst.subPassIdx, dependency);
            }
            Self& DependencyExternalDst(const SubPass& src, SubPassDependency dependency)
            {
                return Dependency(src.subPassIdx, VK_SUBPASS_EXTERNAL, dependency);
            }
            Self& Attachment(uint32_t idx, const AttachmentDescription& desc)
            {
                attachments[idx] = desc;
                return *this;
            }
            Self& AddSubPass(const SubPass& subpass)
            {
                subPasses[subpass.subPassIdx] = subpass;
                return *this;
            }
        public:
            std::vector<AttachmentDescription> attachments;
            std::vector<SubPass> subPasses;
            std::vector<SubPassDependency> dependencies;
        };
        RenderPass(const Config& config)
            : config(config)
        {
            std::vector<VkSubpassDescription> subpassDesc;
            subpassDesc.reserve(config.subPasses.size());
            for (auto& sp : config.subPasses)
                subpassDesc.push_back(sp.BuildDescription());

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = config.attachments.size();
            renderPassInfo.pAttachments = config.attachments.data();
            renderPassInfo.subpassCount = subpassDesc.size();
            renderPassInfo.pSubpasses = subpassDesc.data();
            renderPassInfo.dependencyCount = config.dependencies.size();
            renderPassInfo.pDependencies = config.dependencies.data();

            vkCreateRenderPass(*Device::Instance(), &renderPassInfo, nullptr, &handle) == VkSuccess();
        }
        ~RenderPass()
        {
            vkDestroyRenderPass(*Device::Instance(), handle, nullptr);
        }
    public:
        VkRenderPass handle;
        const Config config;
    };
}
