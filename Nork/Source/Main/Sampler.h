#pragma once
#include "Device.h"

class Sampler
{
public:
	Sampler(VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkFilter filter = VK_FILTER_LINEAR, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR)
	{
        samplerInfo = VkSamplerCreateInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = filter;
        samplerInfo.minFilter = filter;
        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = Device::Instance().PhysicalDeviceProperties().limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        vkCreateSampler(Device::Instance().device, &samplerInfo, nullptr, &handle) == VkSuccess();
	}
    ~Sampler()
    {
        vkDestroySampler(Device::Instance().device, handle, nullptr);
    }
public:
    VkSampler handle;
    VkSamplerCreateInfo samplerInfo;
};