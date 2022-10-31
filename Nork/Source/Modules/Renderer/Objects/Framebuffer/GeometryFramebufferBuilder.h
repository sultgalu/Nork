#pragma once

#include "GeometryFramebuffer.h"
#include "FramebufferBuilder.h"

namespace Nork::Renderer {

	class GeometryFramebufferBuilder : FramebufferBuilder
	{
	public:
		GeometryFramebufferBuilder& Depth(std::shared_ptr<Texture2D> depth)
		{
			this->depth = depth;
			return *this;
		}
		GeometryFramebufferBuilder& Position(TextureFormat position)
		{
			this->position = position;
			return *this;
		}
		GeometryFramebufferBuilder& Normal(TextureFormat normal)
		{
			this->normal = normal;
			return *this;
		}
		GeometryFramebufferBuilder& Diffuse(TextureFormat diffuse)
		{
			this->diffuse = diffuse;
			return *this;
		}
		GeometryFramebufferBuilder& Specular(TextureFormat specular)
		{
			this->specular = specular;
			return *this;
		}
		std::shared_ptr<GeometryFramebuffer> Create();
	private:
		void Validate()
		{
			if (depth == nullptr || position == TextureFormat::None || diffuse == TextureFormat::None ||
				normal == TextureFormat::None || specular == TextureFormat::None)
			{
				std::abort();
			}
		}
		void CreateAttachments();
	private:
		TextureFormat
			position = TextureFormat::None,
			diffuse = TextureFormat::None,
			normal = TextureFormat::None,
			specular = TextureFormat::None;
		std::shared_ptr<Texture2D> depth = nullptr;
	};
}
