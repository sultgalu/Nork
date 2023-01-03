#pragma once
#include "../Image.h"
#include "../Vulkan/Framebuffer.h"
#include "DeviceData.h"

namespace Nork::Renderer
{
template<class ShadowT>
struct ShadowMap
{
	std::shared_ptr<Image> image;
	std::shared_ptr<Vulkan::Framebuffer> fb;
	void SetIndex(uint32_t idx)
	{
		Shadow()->shadMap = idx;
	}
	DeviceDataProxy<ShadowT>& Shadow() { return *shadow; }
	std::shared_ptr<DeviceDataProxy<ShadowT>> shadow;
};
struct DirShadowMap: ShadowMap<Data::DirShadow>
{
	glm::mat4 vp;
};
struct PointShadowMap : ShadowMap<Data::PointShadow>
{
	glm::vec3 position;
};
}