#pragma once

#include "../../State/Capabilities.h"
#include "../../Objects/Framebuffer/Framebuffer.h"
#include "../../Objects/Buffer/Buffer.h"
#include "../../Objects/Shader/Shader.h"
#include "../../Objects/GLManager.h"
#include "../../Model/Model.h"
#include "../../DrawUtils.h"

namespace Nork::Renderer {

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

			geometryFb.Bind().SetViewport().Clear();
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
			geometryFb.Position()->Bind(0);
			geometryFb.Diffuse()->Bind(1);
			geometryFb.Normal()->Bind(2);
			geometryFb.Specular()->Bind(3);

			lightFb.Bind().SetViewport().Clear();
			shader.Use();

			Capabilities::DepthTest().Disable();
			Capabilities::CullFace().Disable();
			DrawUtils::DrawQuad();
			Capabilities::DepthTest().Enable();
		}
	};
}

