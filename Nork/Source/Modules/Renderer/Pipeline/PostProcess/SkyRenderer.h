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
			Capabilities::DepthTest().Enable();
			Capabilities::DepthTest().SetFunc(DepthTestCap::Func::LessOrEqual);
			texture.Bind();
			shader.Use();

			DrawUtils::DrawCube();

			Capabilities::DepthTest().SetFunc(DepthTestCap::Func::LessOrEqual); // set back
		}
	};
}