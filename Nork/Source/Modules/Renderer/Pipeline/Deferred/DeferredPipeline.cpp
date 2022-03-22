#include "pch.h"
#include "DeferredPipeline.h"
#include "../../State/Capabilities.h"
#include "../../Objects/Framebuffer/GeometryFramebufferBuilder.h"
#include "../../Objects/Framebuffer/LightFramebufferBuilder.h"
#include "../../DrawUtils.h"

namespace Nork::Renderer {
	DeferredPipeline::DeferredPipeline(std::shared_ptr<Shader> geomatryShader, std::shared_ptr<Shader> lightShader, uint32_t width, uint32_t height)
		: geomatryShader(geomatryShader), lightShader(lightShader)
	{
		using enum Renderer::TextureFormat;
		geometryFb = GeometryFramebufferBuilder()
			.Width(width).Height(height)
			.Position(RGB16F)
			.Normal(RGB16F)
			.Diffuse(RGB16F)
			.Specular(RGBA16F)
			.Depth(Depth16)
			.Create();
		lightFb = LightFramebufferBuilder()
			.DepthTexture(geometryFb->Depth())
			.ColorFormat(Renderer::TextureFormat::RGBA16F)
			.Create();
	}
	void DeferredPipeline::GeometryPass(DrawableIterator iterator)
	{
		geometryFb->Bind().SetViewport().Clear();
		geomatryShader->Use();

		Capabilities()
			.Enable().DepthTest().CullFace()
			.Disable().Blend(); //.CullFace();

		iterator([&](const IDrawable& drawable)
			{
				drawable.Draw(*geomatryShader);
			});
	}
	void DeferredPipeline::LightPass()
	{
		geometryFb->Position()->Bind(0);
		geometryFb->Diffuse()->Bind(1);
		geometryFb->Normal()->Bind(2);
		geometryFb->Specular()->Bind(3);

		lightFb->Bind().SetViewport().Clear();
		lightShader->Use();

		Capabilities()
			.Disable().DepthTest().Blend();
		DrawUtils::DrawQuad();
	}
}