#pragma once
#include "../Image.h"
#include "../Vulkan/Framebuffer.h"
#include "DeviceData.h"
#include "../Data/Lights.h"

namespace Nork::Renderer
{
template<class ShadowT>
struct ShadowMap
{
	std::shared_ptr<Image> image;
	std::shared_ptr<Vulkan::Framebuffer> fb;
	DeviceDataProxy<ShadowT>& Shadow() { return *shadow; }
	std::shared_ptr<DeviceDataProxy<ShadowT>> shadow;
};
struct DirShadowMap: ShadowMap<Data::DirShadow>
{
	DirShadowMap();
	~DirShadowMap();
	void CreateTexture(uint32_t width, uint32_t height);
	glm::mat4 vp;
};
struct PointShadowMap : ShadowMap<Data::PointShadow>
{
	PointShadowMap();
	~PointShadowMap();
	void CreateTexture(uint32_t size);
	glm::vec3 position;
};
}