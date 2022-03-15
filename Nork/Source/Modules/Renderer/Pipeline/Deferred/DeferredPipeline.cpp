#include "pch.h"
#include "DeferredPipeline.h"
#include "../../DrawUtils.h"

namespace Nork::Renderer {

	void DeferredPipeline::GeometryPass(ModelIterator iterator)
	{
		geometryFb->Bind().SetViewport().Clear();
		geomatryShader->Use();

		Capabilities()
			.Enable().DepthTest().CullFace()
			.Disable().Blend();

		iterator([&](Model& model)
			{
				model.Draw(*geomatryShader);
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