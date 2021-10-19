#pragma once

#include "../Utils.h"

namespace Nork::Renderer
{
	class FramebufferBase
	{
	public:
		void Use();
		void Clear(GLenum clearBits);
		void ClearAndUse(GLenum clearBits);
		FramebufferBase GetCurrentInUse();

		inline bool operator==(const FramebufferBase& other)
		{
			return other.fbo == fbo;
		}

		static void UseDefault();

		GLuint fbo = 0;
		uint32_t width, height;
	};

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
	class Framebuffer : protected FramebufferBase
	{
		using Self = Framebuffer<Depth, Colors...>;
	public:
		Framebuffer(std::unordered_map<TextureFormat, GLuint> externalAtts, uint32_t width, uint32_t height) 
			: FramebufferBase {.width = width, .height = height}
		{
			auto builder = Utils::Framebuffer::Builder(width, height);

			if constexpr (HasDepth())
			{
				if (externalAtts.contains(Depth))
				{
					builder.AddTexture(externalAtts[Depth], GL_DEPTH_ATTACHMENT);
					depth = externalAtts[Depth];
				}
				else
					builder.AddTexture(&depth, Depth, GL_DEPTH_ATTACHMENT);
			}
			if constexpr (HasColor())
			{
				constexpr TextureFormat formats[] = { Colors... };
				GLenum drawBufs[sizeof...(Colors)];
				for (size_t i = 0; i < sizeof...(Colors); i++)
				{
					if (externalAtts.contains(formats[i]))
					{
						builder.AddTexture(externalAtts[formats[i]], GL_COLOR_ATTACHMENT0 + i);
						colors[i] = externalAtts[formats[i]];
					}
					else
						builder.AddTexture(&colors[i], formats[i], GL_COLOR_ATTACHMENT0 + i);
					drawBufs[i] = GL_COLOR_ATTACHMENT0 + i;
				}
				glDrawBuffers(sizeof...(Colors), drawBufs);
			}
			else
			{
				glDrawBuffer(GL_NONE);
			}

			fbo = builder.GetFramebuffer(); 
		}

		Framebuffer(uint32_t width = 1000, uint32_t height = 1000)
			: FramebufferBase{ .width = width, .height = height }
		{
			auto builder = Utils::Framebuffer::Builder(width, height);

			if constexpr (HasDepth())
			{
				builder.AddTexture(&depth, Depth, GL_DEPTH_ATTACHMENT);
			}

			if constexpr (HasColor())
			{
				constexpr TextureFormat formats[] = { Colors... };
				GLenum drawBufs[sizeof...(Colors)];
				for (size_t i = 0; i < sizeof...(Colors); i++)
				{
					builder.AddTexture(&colors[i], formats[i], GL_COLOR_ATTACHMENT0 + i);
					drawBufs[i] = GL_COLOR_ATTACHMENT0 + i;
				}
				glDrawBuffers(sizeof...(Colors), drawBufs);	
			}
			else
			{
				glDrawBuffer(GL_NONE);
			}

			fbo = builder.GetFramebuffer();
		}

		using FramebufferBase::Use;
		using FramebufferBase::Clear;
		using FramebufferBase::ClearAndUse;

		inline void Clear()
		{
			Clear(clearBits);
		}
		inline void ClearAndUse()
		{
			ClearAndUse(clearBits);
		}

		template < typename = typename std::enable_if<Self::HasDepth()>::type >
		GLuint GetDepthAttachment()
		{
			return depth;
		}
		template < typename = typename std::enable_if<Self::HasColor()>::type >
		GLuint GetColorAttachment(uint32_t idx)
		{
			if (idx >= ColorCount())
			{
				MetaLogger().Error("ColorCount is ", ColorCount(), ", idx is: ", idx);
				return 0;
			}
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
		inline static constexpr GLenum clearBits = (HasColor() ? GL_COLOR_BUFFER_BIT : 0) | (HasDepth() ? GL_DEPTH_BUFFER_BIT : 0);
	protected:
		std::enable_if<HasDepth(), GLuint>::type depth;
		std::enable_if<HasDepth(), std::array<GLuint, ColorCount()>>::type colors = {};
	};
}