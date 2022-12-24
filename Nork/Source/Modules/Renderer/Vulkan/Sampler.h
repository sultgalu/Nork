#pragma once
#include "Device.h"

namespace Nork::Renderer::Vulkan {

    struct SamplerCreateInfo : vk::SamplerCreateInfo
    {
        SamplerCreateInfo(vk::Filter filter = vk::Filter::eLinear, vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat)
        {
            this->magFilter = filter;
            this->minFilter = filter;
            this->addressModeU = addressMode;
            this->addressModeV = addressMode;
            this->addressModeW = addressMode;

            this->mipmapMode = vk::SamplerMipmapMode::eLinear;
            this->mipLodBias = 0.0f;
            this->minLod = 0.0f;
            this->maxLod = 0.0f;

            this->anisotropyEnable = true;
            this->maxAnisotropy = PhysicalDevice::Instance().physicalDeviceProperties.limits.maxSamplerAnisotropy;

            this->borderColor = vk::BorderColor::eIntOpaqueBlack;;
            this->unnormalizedCoordinates = true;
            this->compareEnable = true;
            this->compareOp = vk::CompareOp::eAlways;

            this->unnormalizedCoordinates = false;
        }
    };
    class Sampler: public vk::raii::Sampler
    {
    public:
        Sampler(const SamplerCreateInfo& createInfo = SamplerCreateInfo())
            : vk::raii::Sampler(Device::Instance(), createInfo), createInfo(createInfo)
        {
        }
    public:
        SamplerCreateInfo createInfo;
    };
}