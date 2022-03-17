#pragma once
#include "Texture.h"

namespace Nork::Renderer {
	class TextureBuilder
	{
	public:
		TextureBuilder& Params(TextureParams params)
		{
			this->params = params;
			return *this;
		}
		TextureBuilder& Attributes(TextureAttributes attributes)
		{
			this->attributes = attributes;
			return *this;
		}
		std::shared_ptr<Texture2D> Create2DWithData(void* data)
		{
			data2D = data;
			return Create2D();
		}
		std::shared_ptr<TextureCube> CreateCubeWithData(const std::array<void*, 6>& data)
		{
			dataCube = data;
			return CreateCube();
		}
		std::shared_ptr<Texture2D> Create2DEmpty()
		{
			data2D = nullptr;
			return Create2D();
		}
		std::shared_ptr<TextureCube> CreateCubeEmpty()
		{
			dataCube = { nullptr };
			return CreateCube();
		}
	private:
		std::shared_ptr<Texture2D> Create2D();
		std::shared_ptr<TextureCube> CreateCube();
		void Create(bool cube);
		void Validate()
		{
			if (attributes.width == 0 || attributes.height == 0 || attributes.format == TextureFormat::None
				|| params.filter == TextureFilter::None || params.wrap == TextureWrap::None)
			{
				std::abort();
			}
		}
		void SetData(bool cube);
		void SetParams(bool cube);
	private:
		GLuint handle;
		std::array<void*, 6> dataCube;
		void* data2D = nullptr;
		TextureParams params;
		TextureAttributes attributes;
	};
}