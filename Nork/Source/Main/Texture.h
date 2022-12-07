#pragma once

#include "Modules/Renderer/LoadUtils.h"

class Texture
{
public:
    void createTextureImage()
{
    auto image = Nork::Renderer::LoadUtils::LoadImage("texture.jpg", true);
    auto imgSize = image.data.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imgSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);
    void* data;
    vkMapMemory(device.device, stagingBufferMemory, 0, imgSize, 0, &data);
    memcpy(data, image.data.data(), imgSize);
    vkUnmapMemory(device.device, stagingBufferMemory);

    createImage(image.width, image.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, image.width, image.height);
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device.device, stagingBuffer, nullptr);
    vkFreeMemory(device.device, stagingBufferMemory, nullptr);
}
public:
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
};