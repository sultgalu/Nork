#include "DeferredPipeline.h"
#include "../../State/Capabilities.h"
#include "../../Objects/Framebuffer/GeometryFramebufferBuilder.h"
#include "../../Objects/Framebuffer/LightFramebufferBuilder.h"
#include "../../DrawUtils.h"

namespace Nork::Renderer {
	DeferredPipeline::DeferredPipeline(std::shared_ptr<Texture2D> depth)
	{
		using enum Renderer::TextureFormat;
		geometryFb = GeometryFramebufferBuilder()
			.Position(RGB16F)
			.Normal(RGB16F)
			.Diffuse(RGB16F)
			.Specular(RGB16F)
			.Depth(depth)
			.Create();
	}
	void DeferredPipeline::GeometryPass(Shader& shader, const std::vector<DrawCommandMultiIndirect>& drawCommands)
	{
		geometryFb->Bind().SetViewport().ClearColor();
		shader.Use();

		Capabilities()
			.Enable().DepthTest().CullFace()
			.Disable().Blend(); //.CullFace();

		for (auto& command : drawCommands)
		{
			command.Draw(shader);
		}
	}
	void DeferredPipeline::LightPass(Shader& shader, MainFramebuffer& fb)
	{
		geometryFb->Position()->Bind(0);
		geometryFb->Diffuse()->Bind(1);
		geometryFb->Normal()->Bind(2);
		geometryFb->Specular()->Bind(3);

		fb.Bind().SetViewport();
		shader.Use();

		Capabilities()
			.Disable().DepthTest().Blend();
		DrawUtils::DrawQuad();
	}
}