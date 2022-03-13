#include "pch.h"
#include "DeferredPipeline.h"

namespace Nork::Renderer {

	void DeferredPipeline::GeometryPass(GeometryFramebuffer& geometryFb, Shader& shader, std::span<Model> models)
	{
		geometryFb.Bind().SetViewport().Clear();
		shader.Use();

		Capabilities()
			.Enable().DepthTest().CullFace()
			.Disable().Blend();
		for (size_t i = 0; i < models.size(); i++)
		{
			models[i].Draw(shader);
		}
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