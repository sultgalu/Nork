#include "Pipeline.h"

namespace Nork::Renderer::Vulkan {

PipelineCreateInfo::PipelineCreateInfo()
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
    data->colorBlendState.blendConstants = std::array{ 1.0f, 1.0f, 1.0f, 1.0f }; // Optional

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
void PipelineCreateInfo::UpdateArrayPointers()
{
    this->setStages(data->shaderStages);
    data->colorBlendState.setAttachments(data->colorBlendAttachments);
}
PipelineCreateInfo& PipelineCreateInfo::AddShader(const ShaderModule& shad, const char* entry)
{
    data->shaderStages.push_back(vk::PipelineShaderStageCreateInfo({}, shad.stage, *shad, entry));
    UpdateArrayPointers();
    return *this;
}
PipelineCreateInfo& PipelineCreateInfo::VertexInputHardCoded()
{
    data->vertexInput.setVertexAttributeDescriptions({});
    data->vertexInput.setVertexBindingDescriptions({});
    return *this;
}
PipelineCreateInfo& PipelineCreateInfo::InputAssembly(vk::PrimitiveTopology topology)
{
    data->inputAssembly.topology = topology;
    data->inputAssembly.primitiveRestartEnable = false;
    return *this;
}
PipelineCreateInfo& PipelineCreateInfo::Rasterization(bool cullFace, vk::FrontFace frontFace)
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
PipelineCreateInfo& PipelineCreateInfo::Multisampling(vk::SampleCountFlagBits count)
{
    data->multisampling.sampleShadingEnable = false;
    data->multisampling.rasterizationSamples = count;
    data->multisampling.minSampleShading = 1.0f; // Optional
    data->multisampling.pSampleMask = nullptr; // Optional
    data->multisampling.alphaToCoverageEnable = false; // Optional
    data->multisampling.alphaToOneEnable = false; // Optional
    return *this;
}
PipelineCreateInfo& PipelineCreateInfo::ColorBlend(uint32_t attachmentCount, bool blend)
{
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    {
        using enum vk::ColorComponentFlagBits;
        colorBlendAttachment.colorWriteMask = eR | eG | eB | eA;
    }
    colorBlendAttachment.blendEnable = blend;
    colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
    colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
    colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
    data->colorBlendAttachments = std::vector<vk::PipelineColorBlendAttachmentState>(attachmentCount, colorBlendAttachment);

    UpdateArrayPointers();
    return *this;
}
PipelineCreateInfo& PipelineCreateInfo::DepthStencil(bool depthTest, vk::CompareOp op)
{
    return DepthStencil(depthTest, depthTest, op);
}
PipelineCreateInfo& PipelineCreateInfo::DepthStencil(bool depthTest, bool depthWrite, vk::CompareOp op)
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
PipelineCreateInfo& PipelineCreateInfo::RenderPass(vk::RenderPass renderPass, uint32_t subpass)
{
    this->renderPass = renderPass;
    this->subpass = subpass;
    return *this;
}
PipelineCreateInfo& PipelineCreateInfo::Layout(vk::PipelineLayout layout)
{
    this->layout = layout;
    return *this;
}
PipelineCreateInfo& PipelineCreateInfo::AdditionalDynamicStates(const std::vector<vk::DynamicState>& val) {
    data->dynamicStates.insert(data->dynamicStates.end(), val.begin(), val.end());
    data->dynamicState.setDynamicStates(data->dynamicStates);
    return *this;
}
}