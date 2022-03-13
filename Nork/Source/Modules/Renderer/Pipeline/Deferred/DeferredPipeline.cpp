#include "pch.h"
#include "DeferredPipeline.h"

namespace Nork::Renderer {

	void DeferredPipeline::GeometryPass(GeometryFramebuffer& geometryFb, Shader& shader, ModelIterator iterator)
	{
		geometryFb.Bind().SetViewport().Clear();
		shader.Use();

		Capabilities()
			.Enable().DepthTest().CullFace()
			.Disable().Blend();

		iterator([&](Model& model)
			{
				model.Draw(shader);
			});
	}
	void DeferredPipeline::LightPass(GeometryFramebuffer& geometryFb, LightFramebuffer& lightFb, Shader& shader)
	{
		geometryFb.Position()->Bind(0);
		geometryFb.Diffuse()->Bind(1);
		geometryFb.Normal()->Bind(2);
		geometryFb.Specular()->Bind(3);

		lightFb.Bind().SetViewport().Clear();
		shader.Use();

		Capabilities()
			.Disable().DepthTest().Blend();
		DrawUtils::DrawQuad();
	}
}