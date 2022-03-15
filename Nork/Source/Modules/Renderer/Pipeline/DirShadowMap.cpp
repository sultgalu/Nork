#include "pch.h"
#include "DirShadowMapRenderer.h"
#include "../State/Capabilities.h"
#include "../Config.h"

namespace Nork::Renderer {

	void DirShadowMap::Render(const DirLight& light, const DirShadow& shadow, ModelIterator iterator)
	{
		framebuffer->Bind().SetViewport().Clear();
		shader->Use().SetMat4("VP", shadow.VP);

		Capabilities()
			.Enable().DepthTest().CullFace();

		iterator([&](Model& model)
			{
				model.DrawTextureless(*shader);
			});
	}

	void DirShadowMap::Bind(const DirShadow& shadow)
	{
		framebuffer->GetAttachments().depth->Bind2D(shadow.idx + Config::LightData::dirShadowBaseIndex);
	}
	std::shared_ptr<Texture2D> DirShadowMap::Get()
	{
		return std::static_pointer_cast<Texture2D>(framebuffer->GetAttachments().depth);
	}
}
