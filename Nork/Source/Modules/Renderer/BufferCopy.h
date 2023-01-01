#pragma once

namespace Nork::Renderer {

struct BufferCopy
{
    const void* data;
    vk::DeviceSize size;
    vk::DeviceSize dstOffset;
};
}