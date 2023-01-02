#pragma once
#include "../Image.h"
#include "../Vulkan/Framebuffer.h"
#include "DeviceData.h"

namespace Nork::Renderer
{
struct ShadowMap
{
	std::shared_ptr<Image> image;
	std::shared_ptr<Vulkan::Framebuffer> fb;
	glm::mat4 vp;
	void SetIndex(uint32_t idx)
	{
		Shadow()->shadMap = idx;
	}
	DeviceDataProxy<Data::DirShadow>& Shadow() { return *shadow; }
	std::shared_ptr<DeviceDataProxy<Data::DirShadow>> shadow;
};

}