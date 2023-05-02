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
        PipelineCreateInfo();
        void UpdateArrayPointers();
        Self& AddShader(const ShaderModule& shad, const char* entry = "main");
        template<VertexConcept T>
        Self& VertexInput()
        {
            data->vertexBindings = T::getBindingDescription();
            data->vertexAttributes = T::getAttributeDescriptions();
            data->vertexInput.setVertexAttributeDescriptions(data->vertexAttributes);
            data->vertexInput.setVertexBindingDescriptions(data->vertexBindings);
            return *this;
        }
        Self& VertexInputHardCoded();
        Self& InputAssembly(vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList);
        Self& Rasterization(bool cullFace, vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise);
        Self& Multisampling(vk::SampleCountFlagBits count = vk::SampleCountFlagBits::e1);
        Self& ColorBlend(uint32_t attachmentCount, bool blend = false);
        Self& DepthStencil(bool depthTest, vk::CompareOp op = vk::CompareOp::eLess);
        Self& DepthStencil(bool depthTest, bool depthWrite, vk::CompareOp op = vk::CompareOp::eLess);
        Self& RenderPass(vk::RenderPass renderPass, uint32_t subpass);
        Self& Layout(vk::PipelineLayout layout);
        Self& AdditionalDynamicStates(const std::vector<vk::DynamicState>& val);
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

    class ComputePipeline : public  vk::raii::Pipeline
    {
    public:
        ComputePipeline(vk::ComputePipelineCreateInfo& createInfo)
            : vk::raii::Pipeline(Device::Instance(), nullptr, createInfo),
            createInfo(std::move(createInfo))
        {}
    public:
        vk::ComputePipelineCreateInfo createInfo;
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