#pragma once

#include "ShaderModule.h"

template<class T>
concept VertexConcept = requires()
{
    T::getBindingDescription();
    T::getAttributeDescriptions();
};

class PipelineInfo
{
public:
    using Self = PipelineInfo;
    Self& AddShader(const ShaderModule& shad, const char* entry = "main")
    {
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage = shad.stage;
        info.module = shad.handle;
        info.pName = entry;
        shaderStage.push_back(info);
        return *this;
    }
    template<VertexConcept T>
    Self& VertexInput()
    {
        auto& bindingDescription = T::getBindingDescription();
        auto& attributeDescriptions = T::getAttributeDescriptions();
        vertexInput = VkPipelineVertexInputStateCreateInfo{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInput.pVertexBindingDescriptions = &bindingDescription;
        vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();
        return *this;
    }
    Self& InputAssembly(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    {
        inputAssembly = VkPipelineInputAssemblyStateCreateInfo{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        return *this;
    }
    Self& Rasterization()
    {
        rasterization = VkPipelineRasterizationStateCreateInfo{};
        rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization.depthClampEnable = VK_FALSE; // clamp frags outside of near/far plane, eg. for shadowmaps 
        rasterization.rasterizerDiscardEnable = VK_FALSE; // disables output to fragment shader
        rasterization.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization.lineWidth = 1.0f;
        rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization.depthBiasEnable = VK_FALSE;
        rasterization.depthBiasConstantFactor = 0.0f; // Optional
        rasterization.depthBiasClamp = 0.0f; // Optional
        rasterization.depthBiasSlopeFactor = 0.0f; // Optional
        return *this;
    }
    Self& Multisampling()
    {
        multisampling = VkPipelineMultisampleStateCreateInfo{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional
        return *this;
    }
    Self& ColorBlend()
    {
        colorBlendAttachment = VkPipelineColorBlendAttachmentState{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = false;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        colorBlending = VkPipelineColorBlendStateCreateInfo{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        return *this;
    }
    Self& Layout(const VkDescriptorSetLayout& descriptorSetLayout)
    {
        pipelineLayoutInfo = VkPipelineLayoutCreateInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
        return *this;
    }
    Self& DepthStencil(bool depthTest, VkCompareOp op = VK_COMPARE_OP_LESS)
    {
        return DepthStencil(depthTest, depthTest, op);
    }
    Self& DepthStencil(bool depthTest, bool depthWrite, VkCompareOp op = VK_COMPARE_OP_LESS)
    {
        depthStencil = VkPipelineDepthStencilStateCreateInfo{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = depthTest;
        depthStencil.depthWriteEnable = depthWrite;
        depthStencil.depthCompareOp = op;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional
        return *this;
    }
public:
    std::vector<VkPipelineShaderStageCreateInfo> shaderStage;
    VkPipelineVertexInputStateCreateInfo vertexInput;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineRasterizationStateCreateInfo rasterization;
    VkPipelineMultisampleStateCreateInfo multisampling;

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlending;
    VkPipelineDepthStencilStateCreateInfo depthStencil;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
};

class Pipeline
{
public:
    Pipeline(PipelineInfo createInfo, const VkRenderPass& renderPass)
    {
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = createInfo.shaderStage.size();
        pipelineInfo.pStages = createInfo.shaderStage.data();
        pipelineInfo.pVertexInputState = &createInfo.vertexInput;
        pipelineInfo.pInputAssemblyState = &createInfo.inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &createInfo.rasterization;
        pipelineInfo.pMultisampleState = &createInfo.multisampling;
        pipelineInfo.pDepthStencilState = &createInfo.depthStencil;
        pipelineInfo.pColorBlendState = &createInfo.colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;

        vkCreatePipelineLayout(Device::Instance().device, &createInfo.pipelineLayoutInfo, nullptr, &layoutHandle) == VkSuccess();

        pipelineInfo.layout = layoutHandle;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional, VK_PIPELINE_CREATE_DERIVATIVE_BIT = true
        pipelineInfo.basePipelineIndex = -1; // Optional, VK_PIPELINE_CREATE_DERIVATIVE_BIT = true
        vkCreateGraphicsPipelines(Device::Instance().device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &handle) == VkSuccess();
    }
    ~Pipeline()
    {
        vkDestroyPipeline(Device::Instance().device, handle, nullptr);
        vkDestroyPipelineLayout(Device::Instance().device, layoutHandle, nullptr);
    }
public:
    VkPipelineLayout layoutHandle;
    VkPipeline handle;
private:
    Pipeline() = default;
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

        vkCreateShaderModule(Device::Instance().device, &createInfo, nullptr, &handle) == VkSuccess();

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