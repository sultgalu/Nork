#include "pch.h"
#include "DefaultResources.h"
#include "../Utils.h"

namespace Nork::Renderer::Resource
{
	void InitDefaultTextures()
	{
		using namespace Utils::Texture;
		float diff[]{ 1.0f, 1.0f, 1.0f, 1.0f };
		float norm[]{ 0.5f, 0.5f, 1.0f };
		float refl[]{ 0.0f };
		float rough[]{ 1.0f };
		DefaultResources::textures[static_cast<uint8_t>(Data::TextureType::Diffuse)] = CreateTexture2D(1, 1, Format::RGBA, 1, false, diff);
		DefaultResources::textures[static_cast<uint8_t>(Data::TextureType::Normal)] = CreateTexture2D(1, 1, Format::RGB, 1, false, norm);
		DefaultResources::textures[static_cast<uint8_t>(Data::TextureType::Reflection)] = CreateTexture2D(1, 1, Format::R8, 1, false, refl);
		DefaultResources::textures[static_cast<uint8_t>(Data::TextureType::Roughness)] = CreateTexture2D(1, 1, Format::R8, 1, false, rough);
	}

	void DefaultResources::Init()
	{
		InitDefaultTextures();
	}
	void DefaultResources::Free()
	{
	}
}


