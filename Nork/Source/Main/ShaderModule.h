#pragma once

#include "Device.h"

class ShaderModule
{
public:
    ShaderModule(const std::vector<uint32_t>& code, VkShaderStageFlagBits stage)
        : stage(stage)
    {
        createInfo = VkShaderModuleCreateInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = code.data();

        vkCreateShaderModule(Device::Instance().device, &createInfo, nullptr, &handle) == VkSuccess();
    }
    ~ShaderModule()
    {
        vkDestroyShaderModule(Device::Instance().device, handle, nullptr);
    }
public:
    VkShaderModule handle;
    VkShaderModuleCreateInfo createInfo;
    VkShaderStageFlagBits stage;
};
