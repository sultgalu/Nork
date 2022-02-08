#pragma once

#include "../../State/Capabilities.h"
#include "../../Objects/Framebuffer/Framebuffer.h"
#include "../../Objects/Buffer/Buffer.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Model/Model.h"

namespace Nork::Renderer2 {
	class GeometryFramebuffer: public Framebuffer
	{
	public:
		GeometryFramebuffer& CreateTextures(uint32_t width, uint32_t height,
			TextureFormat depth, TextureFormat position, TextureFormat diffuse,
			TextureFormat normal, TextureFormat specular)
		{
			auto attachments = FramebufferAttachments()
				.Color(Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = width, .height = height, .format = position }), 0)
				.Color(Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = width, .height = height, .format = diffuse }), 1)
				.Color(Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = width, .height = height, .format = normal }), 2)
				.Color(Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = width, .height = height, .format = specular }), 3)
				.Depth(Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = width, .height = height, .format = depth }));

			SetAttachments(attachments);
			return *this;
		}
		Texture2D& Position()
		{
			return attachments.colors[0].first;
		}
		Texture2D& Diffuse()
		{
			return attachments.colors[1].first;
		}
		Texture2D& Normal()
		{
			return attachments.colors[2].first;
		}
		Texture2D& Specular()
		{
			return attachments.colors[3].first;
		}
		Texture2D& Depth()
		{
			return attachments.depth.value();
		}
	};

	class LightFramebuffer : public Framebuffer
	{
	public:
		LightFramebuffer& CreateTextures(uint32_t width, uint32_t height, TextureFormat color, TextureFormat depth)
		{
			auto attachments = FramebufferAttachments()
				.Color(Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = width, .height = height, .format = color }), 0)
				.Depth(Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = width, .height = height, .format = depth }));

			SetAttachments(attachments);
			return *this;
		}
		LightFramebuffer& CreateTextures(GeometryFramebuffer& gFb, TextureFormat color)
		{
			auto attachments = FramebufferAttachments()
				.Color(Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = gFb.Diffuse().Attributes().width, .height = gFb.Diffuse().Attributes().height, .format = color }), 0)
				.Depth(gFb.Depth());

			SetAttachments(attachments);
			return *this;
		}
		Texture2D& Color()
		{
			return attachments.colors[0].first;
		}
		Texture2D& Depth()
		{
			return attachments.depth.value();
		}
	};

	class DeferredPipeline
	{
	public:
		static void GeometryPass(GeometryFramebuffer& geometryFb, Shader& shader, std::span<Model> models)
		{
			Capabilities::DepthTest().Enable();
			Capabilities::DepthTest().SetFunc(DepthTestCap::Func::Less);
			Capabilities::CullFace().Enable();
			Capabilities::CullFace().SetFace(CullFaceCap::Face::Back);
			Capabilities::Blend().Disable();

			geometryFb.Bind().Clear();
			shader.Use();

			for (size_t i = 0; i < models.size(); i++)
			{
				shader.SetMat4("model", models[i].modelMatrix);
				for (int j = 0; j < models[i].meshes.size(); j++)
				{
					models[i].meshes[j].BindTextures();
					models[i].meshes[j].Draw();
				};
			}
		}
		static void LightPass(GeometryFramebuffer& geometryFb, LightFramebuffer& lightFb, Shader& shader)
		{
			Capabilities::DepthTest().Disable();
			geometryFb.Position().Bind(0);
			geometryFb.Diffuse().Bind(1);
			geometryFb.Normal().Bind(2);
			geometryFb.Specular().Bind(3);

			shader.Use();

		}
	};
}

