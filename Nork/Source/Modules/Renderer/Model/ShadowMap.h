#pragma once

#include "../Data/Lights.h"
#include "../Objects/Framebuffer/Framebuffer.h"
#include "../Storage/SmartMappedBuffer.h"

namespace Nork::Renderer {
	struct DirShadowMap
	{
		DirShadowMap() = default;
		DirShadowMap(UBO<Data::DirLight>::Element light, UBO<Data::DirShadow>::Element shadow)
			: light(light), shadow(shadow) 
		{}
		UBO<Data::DirLight>::Element light;
		UBO<Data::DirShadow>::Element shadow;
		std::shared_ptr<Framebuffer> fb;

		std::shared_ptr<Texture2D> Texture();

		void SetFramebuffer(uint32_t w, uint32_t h, TextureFormat depth);
	};
	struct PointShadowMap
	{
		PointShadowMap() = default;
		PointShadowMap(UBO<Data::PointLight>::Element light, UBO<Data::PointShadow>::Element shadow)
			: light(light), shadow(shadow)
		{}
		UBO<Data::PointLight>::Element light;
		UBO<Data::PointShadow>::Element shadow;
		std::shared_ptr<Framebuffer> fb;

		std::shared_ptr<TextureCube> Texture();

		void SetFramebuffer(uint32_t res, TextureFormat depth);

	};
}