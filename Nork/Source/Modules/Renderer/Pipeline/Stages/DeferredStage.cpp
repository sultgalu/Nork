#include "DeferredStage.h"
#include "../../State/Capabilities.h"
#include "../../Objects/Framebuffer/GeometryFramebufferBuilder.h"
#include "../../DrawUtils.h"

namespace Nork::Renderer {
	bool DeferredStage::Execute(Framebuffer& source, Framebuffer& destination)
	{
		GeometryPass();
		LightPass(source);
		return false;
	}
	DeferredStage::DeferredStage(std::shared_ptr<Texture2D> depth, std::shared_ptr<Shader> gShader, std::shared_ptr<Shader> lShader, DrawCommand* drawCommand)
		: gShader(gShader), lShader(lShader), drawCommand(drawCommand)
	{
		using enum Renderer::TextureFormat;
		geometryFb = GeometryFramebufferBuilder()
			.Position(RGBA16F)
			.Normal(RGB16F)
			.Diffuse(RGB16F)
			.Specular(RGB16F)
			.Depth(depth)
			.Create();
	}
	void DeferredStage::GeometryPass()
	{
		geometryFb->Bind().SetViewport().ClearColor();
		gShader->Use();

		Capabilities()
			.Enable().DepthTest().CullFace()
			.Disable().Blend(); //.CullFace();

		drawCommand->operator()();
	}
	void DeferredStage::LightPass(Framebuffer& fb)
	{
		geometryFb->Position()->Bind(0);
		geometryFb->Diffuse()->Bind(1);
		geometryFb->Normal()->Bind(2);
		geometryFb->Specular()->Bind(3);

		fb.Bind().SetViewport();
		lShader->Use();

		Capabilities()
			.Disable().DepthTest().Blend();
		DrawUtils::DrawQuad();
	}
}