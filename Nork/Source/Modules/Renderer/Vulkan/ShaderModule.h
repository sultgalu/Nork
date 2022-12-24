#pragma once
#include "Device.h"

namespace Nork::Renderer::Vulkan {
    class ShaderModule: public vk::raii::ShaderModule
    {
    public:
        ShaderModule(const std::vector<uint32_t>& code, vk::ShaderStageFlagBits stage)
            : stage(stage), vk::raii::ShaderModule(Device::Instance(),
                vk::ShaderModuleCreateInfo({}, code.size() * sizeof(code[0]), code.data()))
        {}
    public:
        vk::ShaderStageFlagBits stage;
    };
}
