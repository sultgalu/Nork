#pragma once

#include "ShaderModule.h"
#include "RenderPass.h"

namespace Nork::Renderer::Vulkan {
    template<class T>
    concept VertexConcept = requires()
    {
        T::getBindingDescription();
        T::getAttributeDescriptions();
    };

    struct PipelineLayoutCreateInfo : vk::PipelineLayoutCreateInfo
    {
        PipelineLayoutCreateInfo(PipelineLayoutCreateInfo&&) = default;
        PipelineLayoutCreateInfo(const std::vector<vk::DescriptorSetLayout>& descriptorSetLayout,
            const std::vector<vk::PushConstantRange>& pushConstantRanges)
        {
            this->descriptorSetLayout = descriptorSetLayout;
            this->pushConstants = pushConstantRanges;
            this->setSetLayouts(descriptorSetLayout);
            if (pushConstants.size() > 0)
                this->setPushConstantRanges(pushConstants); // Optional
        }
        std::vector<vk::DescriptorSetLayout> descriptorSetLayout;
        std::vector<vk::PushConstantRange> pushConstants;
    };

    struct PipelineCreateInfo: vk::GraphicsPipelineCreateInfo
    {
    public:
        using Self = PipelineCreateInfo;
        PipelineCreateInfo()
        {
            data->dynamicStates = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor
            };

            data->dynamicState.setDynamicStates(data->dynamicStates);
            data->viewportState.viewportCount = 1;
            data->viewportState.scissorCount = 1;

            data->colorBlendState.logicOpEnable = VK_FALSE;
            data->colorBlendState.logicOp = vk::LogicOp::eCopy; // Optional
            data->colorBlendState.blendConstants = std::array {0.f, 0.f, 0.f, 0.f}; // Optional

            this->pVertexInputState = &data->vertexInput;
            this->pInputAssemblyState = &data->inputAssembly;
            this->pViewportState = &data->viewportState;
            this->pRasterizationState = &data->rasterization;
            this->pMultisampleState = &data->multisampling;
            this->pDepthStencilState = &data->depthStencil;
            this->pDynamicState = &data->dynamicState;
            this->pColorBlendState = &data->colorBlendState;
            UpdateArrayPointers();

            this->basePipelineHandle = nullptr; // Optional, VK_PIPELINE_CREATE_DERIVATIVE_BIT = true
            this->basePipelineIndex = -1; // Optional, VK_PIPELINE_CREATE_DERIVATIVE_BIT = true
        }
        void UpdateArrayPointers()
        {
            this->setStages(data->shaderStages);
            data->colorBlendState.setAttachments(data->colorBlendAttachments);
        }
        Self& AddShader(const ShaderModule& shad, const char* entry = "main")
        {
            data->shaderStages.push_back(vk::PipelineShaderStageCreateInfo({}, shad.stage, *shad, entry));
            UpdateArrayPointers();
            return *this;
        }
        template<VertexConcept T>
        Self& VertexInput()
        {
            data->vertexBindings = T::getBindingDescription();
            data->vertexAttributes = T::getAttributeDescriptions();
            data->vertexInput.setVertexAttributeDescriptions(data->vertexAttributes);
            data->vertexInput.setVertexBindingDescriptions(data->vertexBindings);
            return *this;
        }
        Self& VertexInputHardCoded()
        {
            data->vertexInput.setVertexAttributeDescriptions({});
            data->vertexInput.setVertexBindingDescriptions({});
            return *this;
        }
        Self& InputAssembly(vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList)
        {
            data->inputAssembly.topology = topology;
            data->inputAssembly.primitiveRestartEnable = false;
            return *this;
        }
        Self& Rasterization(bool cullFace, vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise)
        {
            data->rasterization.depthClampEnable = false; // clamp frags outside of near/far plane, eg. for shadowmaps 
            data->rasterization.rasterizerDiscardEnable = false; // disables output to fragment shader
            data->rasterization.polygonMode = vk::PolygonMode::eFill;
            data->rasterization.lineWidth = 1.0f;
            data->rasterization.cullMode = cullFace ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone;
            data->rasterization.frontFace = frontFace;
            data->rasterization.depthBiasEnable = false;
            data->rasterization.depthBiasConstantFactor = 0.0f; // Optional
            data->rasterization.depthBiasClamp = 0.0f; // Optional
            data->rasterization.depthBiasSlopeFactor = 0.0f; // Optional
            return *this;
        }
        Self& Multisampling()
        {
            data->multisampling.sampleShadingEnable = false;
            data->multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
            data->multisampling.minSampleShading = 1.0f; // Optional
            data->multisampling.pSampleMask = nullptr; // Optional
            data->multisampling.alphaToCoverageEnable = false; // Optional
            data->multisampling.alphaToOneEnable = false; // Optional
            return *this;
        }
        Self& ColorBlend(uint32_t attachmentCount)
        {
            vk::PipelineColorBlendAttachmentState colorBlendAttachment;
            {
                using enum vk::ColorComponentFlagBits;
                colorBlendAttachment.colorWriteMask = eR | eG | eB | eA;
            }
            colorBlendAttachment.blendEnable = false;
            colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne; // Optional
            colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
            colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd; // Optional
            colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne; // Optional
            colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
            colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd; // Optional
            data->colorBlendAttachments = std::vector<vk::PipelineColorBlendAttachmentState>(attachmentCount, colorBlendAttachment);

            UpdateArrayPointers();
            return *this;
        }
        Self& DepthStencil(bool depthTest, vk::CompareOp op = vk::CompareOp::eLess)
        {
            return DepthStencil(depthTest, depthTest, op);
        }
        Self& DepthStencil(bool depthTest, bool depthWrite, vk::CompareOp op = vk::CompareOp::eLess)
        {
            data->depthStencil.depthTestEnable = depthTest;
            data->depthStencil.depthWriteEnable = depthWrite;
            data->depthStencil.depthCompareOp = op;
            data->depthStencil.depthBoundsTestEnable = false;
            data->depthStencil.minDepthBounds = 0.0f; // Optional
            data->depthStencil.maxDepthBounds = 1.0f; // Optional
            data->depthStencil.stencilTestEnable = false;
            data->depthStencil.front = vk::StencilOpState(); // Optional
            data->depthStencil.back = vk::StencilOpState(); // Optional
            return *this;
        }
        Self& RenderPass(vk::RenderPass renderPass, uint32_t subpass)
        {
            this->renderPass = renderPass;
            this->subpass = subpass;
            return *this;
        }
        Self& Layout(vk::PipelineLayout layout)
        {
            this->layout = layout;
            return *this;
        }
    public:
        struct Data
        {
            std::vector<vk::DynamicState> dynamicStates;
            vk::PipelineDynamicStateCreateInfo dynamicState;
            vk::PipelineViewportStateCreateInfo viewportState;

            vk::PipelineVertexInputStateCreateInfo vertexInput;
            std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
            std::vector<vk::VertexInputBindingDescription> vertexBindings;

            std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
            vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
            vk::PipelineRasterizationStateCreateInfo rasterization;
            vk::PipelineMultisampleStateCreateInfo multisampling;
            vk::PipelineColorBlendStateCreateInfo colorBlendState;

            std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
            vk::PipelineDepthStencilStateCreateInfo depthStencil;

            vk::DescriptorSetLayout descriptorSetLayout;
            std::vector<vk::PushConstantRange> pushConstants;
        };
        std::unique_ptr<Data> data = std::make_unique<Data>();
    };
    class PipelineLayout : public vk::raii::PipelineLayout
    {
    public:
        PipelineLayout(PipelineLayoutCreateInfo&& createInfo)
            : vk::raii::PipelineLayout(Device::Instance(), createInfo),
            createInfo(std::move(createInfo))
        {}
    public:
        PipelineLayoutCreateInfo createInfo;
    };
    class Pipeline : public  vk::raii::Pipeline
    {
    public:
        Pipeline(PipelineCreateInfo& createInfo)
            : vk::raii::Pipeline(Device::Instance(), nullptr, createInfo),
            createInfo(std::move(createInfo))
        {}
    public:
        PipelineCreateInfo createInfo;
    };

    class Shader
    {
    public:
        Shader(const std::vector<uint32_t>& code, VkShaderStageFlagBits usage)
        {
            createInfo = VkShaderModuleCreateInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size() * sizeof(uint32_t);
            createInfo.pCode = code.data();

            vkCreateShaderModule(*Device::Instance(), &createInfo, nullptr, &handle) == VkSuccess();

            vertShaderStageInfo = VkPipelineShaderStageCreateInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = usage;
            vertShaderStageInfo.module = handle;
            vertShaderStageInfo.pName = "main";
        }
    public:
        VkShaderModule handle;
        VkShaderModuleCreateInfo createInfo;
        VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    };
}