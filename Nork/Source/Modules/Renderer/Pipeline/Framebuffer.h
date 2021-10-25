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
		T == TextureFormat::R32F;

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

	template<TextureFormat T, TextureFormat... Rest>
	static consteval int DepthAttCount()
	{
		if constexpr (sizeof...(Rest) > 0)
		{
			return (isDepthFormat<T> ? 1 : 0) + DepthAttCount<Rest...>();
		}
		else
		{
			return isDepthFormat<T> ? 1 : 0;
		}
	}

	template<TextureFormat... AttachmentFormats>
	requires requires()
	{
		requires sizeof...(AttachmentFormats) > 0; // does 0 case make sense?
		requires DepthAttCount<AttachmentFormats...>() <= 1;
	}
	class Framebuffer : protected FramebufferBase
	{
		using Self = Framebuffer<AttachmentFormats...>;
	public:
		template<std::convertible_to<GLuint>... Textures>
		requires requires () { requires sizeof...(Textures) == 0 || sizeof...(Textures) == sizeof...(AttachmentFormats); }
		Framebuffer(uint32_t width, uint32_t height, Textures... textures) // use 0 for inner generation
			: FramebufferBase{ .width = width, .height = height }
		{
			colors.resize(colorCount);

			auto builder = Utils::Framebuffer::Builder(width, height);
			if constexpr (sizeof...(Textures) == 0)
			{
				GenTextures<0, AttachmentFormats...>(builder);
			}
			else
			{
				std::array<GLuint, sizeof...(Textures)> externals = { textures... };
				AddOrGenTextures<0, AttachmentFormats...>(builder, externals.data());
			}
			UpdateDrawBuffers();
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

		template < typename = typename std::enable_if<Self::hasDepth>::type >
		inline GLuint GetDepthAttachment()
		{
			return depth;
		}
		template < typename = typename std::enable_if<Self::hasColor>::type >
		inline GLuint GetColorAttachment(uint32_t idx)
		{
			if (idx >= colorCount)
			{
				MetaLogger().Error("colorCount is ", colorCount, ", idx is: ", idx,
					"\n\tUse GetExtensionAttachment to access textures added as extensions.");
				return 0;
			}
			return colors[idx];
		}
		inline GLuint GetExtension(uint32_t idx)
		{
			return colors[colorCount + idx];
		}
		inline uint8_t ColorAttForExtension(uint8_t idx) { return colorCount + idx; }
		inline GLuint GetFBO() { return fbo; }
		inline size_t Width()
		{
			return width;
		}
		inline size_t Height()
		{
			return height;
		}
		inline static constexpr uint32_t colorCount = sizeof...(AttachmentFormats) - DepthAttCount<AttachmentFormats...>();
		inline static constexpr bool hasDepth = DepthAttCount<AttachmentFormats...>() == 1;
		inline static constexpr bool hasColor = colorCount > 0;
		inline static constexpr GLenum clearBits = (hasColor ? GL_COLOR_BUFFER_BIT : 0) | (hasDepth ? GL_DEPTH_BUFFER_BIT : 0);
		template<TextureFormat Format>
		GLuint Extend(GLuint tex = 0)
		{
			GLenum attIdx = GL_COLOR_ATTACHMENT0 + colors.size();
			colors.push_back(0);

			if (tex == 0)
			{
				Utils::Framebuffer::Builder(width, height, fbo)
					.AddTexture(&colors.back(), Format, attIdx);
			}
			else
			{
				colors.back() = tex;
				Utils::Framebuffer::Builder(width, height, fbo)
					.AddTexture(colors.back(), attIdx);
			}

			UpdateDrawBuffers();
			return colors.back();
		}
	private:
		template<uint8_t colorIdx, TextureFormat Format, TextureFormat... Rest>
		void AddOrGenTextures(Utils::Framebuffer::Builder& builder, GLuint* external)
		{
			if constexpr (isDepthFormat<Format>)
			{
				if (*external != 0)
				{
					depth = *external;
					builder.AddTexture(depth, GL_DEPTH_ATTACHMENT);
				}
				else
				{
					builder.AddTexture(&depth, Format, GL_DEPTH_ATTACHMENT);
				}
				if constexpr (sizeof...(Rest) > 0)
					AddOrGenTextures<colorIdx, Rest...>(builder, external + 1);
			}
			else
			{
				if (*external != 0)
				{
					colors[colorIdx] = *external;
					builder.AddTexture(colors[colorIdx], GL_COLOR_ATTACHMENT0 + colorIdx);
				}
				else
				{
					builder.AddTexture(&colors[colorIdx], Format, GL_COLOR_ATTACHMENT0 + colorIdx);
				}
				if constexpr (sizeof...(Rest) > 0)
					AddOrGenTextures<colorIdx + 1, Rest...>(builder, external + 1);
			}
		}
		template<uint8_t colorIdx, TextureFormat Format, TextureFormat... Rest>
		void GenTextures(Utils::Framebuffer::Builder& builder)
		{
			if constexpr (isDepthFormat<Format>)
			{
				builder.AddTexture(&depth, Format, GL_DEPTH_ATTACHMENT);
				if constexpr (sizeof...(Rest) > 0)
					GenTextures<colorIdx, Rest...>(builder);
			}
			else
			{
				builder.AddTexture(&colors[colorIdx], Format, GL_COLOR_ATTACHMENT0 + colorIdx);
				if constexpr (sizeof...(Rest) > 0)
					GenTextures<colorIdx + 1, Rest...>(builder);
			}
		}
		void UpdateDrawBuffers()
		{
			if (colors.size() > 0)
			{
				auto buf = std::vector<GLenum>(colors.size());
				for (size_t i = 0; i < buf.size(); i++)
					buf[i] = GL_COLOR_ATTACHMENT0 + i;
				glDrawBuffers(buf.size(), buf.data());
			}
			else
			{
				glDrawBuffer(GL_NONE);
			}
		}
	protected:
		GLuint depth;
		std::vector<GLuint> colors;
	};
}