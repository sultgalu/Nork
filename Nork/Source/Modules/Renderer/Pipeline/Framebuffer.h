#pragma once

#include "../Utils.h"

namespace Nork::Renderer
{
	using TextureFormat = Utils::Texture::Format;

	template<Utils::Texture::Format T>
	concept DepthFormat =
		T == TextureFormat::Depth16		||
		T == TextureFormat::Depth24		||
		T == TextureFormat::Depth32		||
		T == TextureFormat::Depth32F	||
		T == TextureFormat::None;

	template<Utils::Texture::Format T>
	concept ColorFormat =
		T == TextureFormat::RGB		||
		T == TextureFormat::RGBA	||
		T == TextureFormat::RGB16F	||
		T == TextureFormat::RGBA16F	||
		T == TextureFormat::R8		||
		T == TextureFormat::R8I		||
		T == TextureFormat::R32I	||
		T == TextureFormat::R32F	||
		T == TextureFormat::None;

	template<TextureFormat Depth, TextureFormat... Colors> requires DepthFormat<Depth> && ColorFormat<Colors...>
	class Framebuffer
	{
		using Self = Framebuffer<Depth, Colors...>;
	
	public:
		Framebuffer(uint32_t width = 1000, uint32_t height = 1000) : width(width), height(height)
		{
			auto builder = Utils::Framebuffer::Builder(width, height);

			auto ptr = attachments.data();

			if constexpr (HasDepth())
			{
				builder.AddTexture(ptr++, Depth, GL_DEPTH_ATTACHMENT);
			}

			if constexpr (HasColor())
			{
				constexpr std::array<TextureFormat, sizeof...(Colors)> formats = { Colors... };
				for (size_t i = 0; i < formats.size(); i++)
				{
					builder.AddTexture(ptr++, formats[i], GL_COLOR_ATTACHMENT0 + i);
				}
			}

			fbo = builder.GetFramebuffer();
		}

		inline void Use()
		{
			glViewport(0, 0, width, height);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		}
		inline void Clear()
		{
			glClear(GetClearBits());
		}
		inline void ClearAndUse()
		{
			Use();
			Clear();
		}

		template < typename = typename std::enable_if<Self::HasDepth()>::type >
		GLuint GetDepthAttachment()
		{
			return attachments[0];
		}

		template < typename = typename std::enable_if<Self::HasColor()>::type >
		GLuint GetColorAttachment(uint32_t idx)
		{
			return attachments[HasDepth() ? idx + 1 : idx];
		}

		inline static consteval bool HasDepth()
		{
			return Depth != TextureFormat::None;
		}
		inline static consteval bool HasColor()
		{
			constexpr TextureFormat arr[] = { Colors... };
			return arr[0] != TextureFormat::None;
		}
		inline static consteval size_t ColorCount()
		{
			return HasColor() ? sizeof...(Colors) : 0;
		}
	private:
		inline static consteval GLenum GetClearBits()
		{
			return (HasColor() ? GL_COLOR_BUFFER_BIT : 0) | (HasDepth() ? GL_DEPTH_BUFFER_BIT : 0);
		}
	private:
		GLuint fbo = 0;
		std::array<GLuint, (HasDepth() ? 1 : 0) + ColorCount()> attachments = {}; // if has depth it is always the first
		size_t width, height;
	};

	using ShadowFramebuffer = Framebuffer<Utils::Texture::Format::Depth16, Utils::Texture::Format::None>;
}