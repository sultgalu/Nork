#pragma once

#include "../Data/Ligths.h"
#include "../Data/Mesh.h"
#include "../Data/Shader.h"
#include "SharedData.h"
#include "Framebuffer.h"

namespace Nork::Renderer::Pipeline
{
	using SFB = Framebuffer<Utils::Texture::Format::Depth16, Utils::Texture::Format::None>;
	class DirShadowFramebuffer : private SFB
	{
	public:
		using SFB::Framebuffer;
		using SFB::ClearAndUse;
		
		GLuint Texture()
		{
			return depth;
		}
	};

	using SFB = Framebuffer<Utils::Texture::Format::Depth16, Utils::Texture::Format::None>;
	class PointShadowFramebuffer : private SFB
	{
	public:
		PointShadowFramebuffer(uint32_t width, uint32_t height)
			: Framebuffer(CreateTexture(width, height), width, height)
		{
		}
		using SFB::ClearAndUse;

		GLuint Texture()
		{
			return depth;
		}
	private:
		std::unordered_map<Utils::Texture::Format, GLuint> CreateTexture(uint32_t width, uint32_t height)
		{
			using namespace Utils::Texture;
			auto tex = Create<TextureType::Cube>(width, height, Format::Depth16, nullptr, TextureParams::CubeMapParams());
			return std::unordered_map<Format, GLuint> { { Format::Depth16, tex } };
		}
	};

	class LightManager
	{
	public:
		LightManager() = default;
		void Update(std::span<std::pair<Data::DirLight, Data::DirShadow>> dLigthswithShad, std::span<std::pair<Data::PointLight, Data::PointShadow>> pLigthsWithShad,
			std::span<Data::DirLight> dLigths, std::span<Data::PointLight> pLigths);
		void DrawPointShadowMap(const Data::PointLight& light, const Data::PointShadow& shadow, const std::span<Data::Model> models, PointShadowFramebuffer& fb, Data::Shader& pShadowMapShader);
		void DrawDirShadowMap(const Data::DirLight& light, const Data::DirShadow& shadow, const std::span<Data::Model> models, DirShadowFramebuffer& fb, Data::Shader& dShadowMapShader);
	private:
		LightShaderDataStorage data;
	};
}

