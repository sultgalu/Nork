#pragma once

#include "../Utils.h"

namespace Nork::Renderer
{
	using TextureFormat = Utils::Texture::Format;

	template<TextureFormat T>
	inline constexpr bool isDepthFormat =
		T == TextureFormat::Depth16 ||
		T == TextureFormat::Depth24 ||
		T == TextureFormat::Depth32 ||
		T == TextureFormat::Depth32F ||
		T == TextureFormat::None;

	template<TextureFormat T>
	inline constexpr bool isColorFormat =
		T == TextureFormat::RGB ||
		T == TextureFormat::RGBA ||
		T == TextureFormat::RGB16F ||
		T == TextureFormat::RGBA16F ||
		T == TextureFormat::R8 ||
		T == TextureFormat::R8I ||
		T == TextureFormat::R32I ||
		T == TextureFormat::R32F ||
		T == TextureFormat::None;

	template<TextureFormat T, TextureFormat... Rest>
	static consteval bool AreColorFormats()
	{
		if constexpr (sizeof...(Rest) > 0)
		{
			return isColorFormat<T> && AreColorFormats<Rest...>();
		}
		else
		{
			return isColorFormat<T>;
		}
	}

	template<TextureFormat Depth, TextureFormat... Colors>
	requires requires()
	{
		requires isDepthFormat<Depth>;
		requires AreColorFormats<Colors...>();
	}
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
				builder.AddTexture(&depth, Depth, GL_DEPTH_ATTACHMENT);
			}

			if constexpr (HasColor())
			{
				constexpr std::array<TextureFormat, sizeof...(Colors)> formats = { Colors... };
				for (size_t i = 0; i < formats.size(); i++)
				{
					builder.AddTexture(&colors[i], formats[i], GL_COLOR_ATTACHMENT0 + i);
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
			return depth;
		}

		template < typename = typename std::enable_if<Self::HasColor()>::type >
		GLuint GetColorAttachment(uint32_t idx)
		{
			return colors(idx);
		}

		inline static consteval bool HasDepth()
		{
			return Depth != TextureFormat::None;
		}
		inline static consteval bool HasColor()
		{
			constexpr TextureFormat arr[] = {Colors...};
			return arr[0] != TextureFormat::None;
		}
		inline static consteval size_t ColorCount()
		{
			return HasColor() ? sizeof...(Colors) : 0;
		}

		inline size_t Width()
		{
			return width;
		}
		inline size_t Height()
		{
			return height;
		}
	protected:
		inline static consteval GLenum GetClearBits()
		{
			return (HasColor() ? GL_COLOR_BUFFER_BIT : 0) | (HasDepth() ? GL_DEPTH_BUFFER_BIT : 0);
		}
	protected:
		GLuint fbo = 0;
		std::enable_if<HasDepth(), GLuint>::type depth;
		std::enable_if<HasDepth(), std::array<GLuint, ColorCount()>>::type colors = {};
		size_t width, height;
	};

	using ShadowFramebuffer = Framebuffer<Utils::Texture::Format::Depth16, Utils::Texture::Format::None>;
}