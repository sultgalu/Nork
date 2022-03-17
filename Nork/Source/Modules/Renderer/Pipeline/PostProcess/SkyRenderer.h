#pragma once

#include "../../State/Capabilities.h"
#include "../../Objects/Texture/Texture.h"
#include "../../Objects/Shader/Shader.h"
#include "../../DrawUtils.h"

namespace Nork::Renderer {
	class SkyRenderer
	{
	public:
		static void RenderSkybox(TextureCube& texture, Shader& shader)
		{
			Capabilities()
				.Enable().DepthTest(DepthFunc::LessOrEqual)
				.Disable().CullFace();
			texture.Bind();
			shader.Use();
			shader.SetInt("skyBox", 0);

			DrawUtils::DrawCube();
		}
	};
}