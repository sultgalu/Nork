#include "RenderingSystem.h"

namespace Nork{
	static std::shared_ptr<Renderer::GeometryFramebuffer> CreateGeometryFramebuffer(uint32_t width, uint32_t height)
	{
		using enum Renderer::TextureFormat;
		return Renderer::GeometryFramebufferBuilder()
			.Width(width).Height(height)
			.Depth(Depth16)
			.Position(RGB16F)
			.Normal(RGB16F)
			.Diffuse(RGB16F)
			.Specular(RGBA16F)
			.Create();
	}
	static std::shared_ptr<Renderer::LightFramebuffer> CreateLightFramebuffer(std::shared_ptr<Renderer::GeometryFramebuffer> gFb)
	{
		return Renderer::LightFramebufferBuilder()
			.DepthTexture(gFb->Depth())
			.ColorFormat(Renderer::TextureFormat::RGBA16F)
			.Create();
	}
	static Renderer::DeferredPipeline CreateDeferredPipeline(uint32_t width, uint32_t height, std::shared_ptr<Renderer::Shader> gShader, std::shared_ptr<Renderer::Shader> lShader)
	{
		auto fb = CreateGeometryFramebuffer(width, height);
		return Renderer::DeferredPipeline(fb, CreateLightFramebuffer(fb), gShader, lShader);
	}
	Renderer::ModelIterator ModelIterator(entt::registry& reg)
	{
		auto iterator = [&](auto func)
		{
			for (auto [id, dr] : reg.view<Components::Drawable>().each())
			{
				func(dr.model);
			}
		};
		return iterator;
	}
	RenderingSystem::RenderingSystem()
		: deferredPipeline(CreateDeferredPipeline(resolution.x, resolution.y, shaders.gPassShader, shaders.lPassShader))
	{
		lightStateSyncher.Initialize();

		using namespace Renderer;
		for (auto& sm : dirShadowMaps)
		{
			sm = std::make_shared<DirShadowMap>(shaders.dShadowShader, 4000, 4000, TextureFormat::Depth16);
		}
		for (auto& sm : pointShadowMaps)
		{
			sm = std::make_shared<PointShadowMap>(shaders.pShadowShader, 1000, TextureFormat::Depth16);
		}
	}
	void RenderingSystem::UpdateLights(entt::registry& reg)
	{
		using namespace Components;
		auto dLightsWS = reg.view<DirLight, DirShadow>();
		auto pLightsWS = reg.view<PointLight, PointShadow>();
		auto dLights = reg.view<DirLight>(entt::exclude<DirShadow>);
		auto pLights = reg.view<PointLight>(entt::exclude<PointShadow>);

		// ---------------------------
		auto& lights = lightStateSyncher.GetLightState();
		lights.ClearAll();

		for (auto& id : dLightsWS)
		{
			const auto& light = dLightsWS.get(id)._Myfirst._Val;
			const auto& shadow = dLightsWS.get(id)._Get_rest()._Myfirst._Val;
			lights.dirLights.push_back(light);
			lights.dirShadows.push_back(shadow);

			dirShadowMaps[shadow.idx]->Render(light, shadow, ModelIterator(reg));
			dirShadowMaps[shadow.idx]->Bind(shadow);
		}
		for (auto& id : pLightsWS)
		{
			auto& light = pLightsWS.get(id)._Myfirst._Val;
			auto& shadow = pLightsWS.get(id)._Get_rest()._Myfirst._Val;

			lights.pointLights.push_back(light);
			lights.pointShadows.push_back(shadow);

			pointShadowMaps[shadow.idx]->Render(light, shadow, ModelIterator(reg));
			pointShadowMaps[shadow.idx]->Bind(shadow);
		}
		for (auto& id : dLights)
		{
			lights.dirLights.push_back(dLights.get(id)._Myfirst._Val);
		}
		for (auto& id : pLights)
		{
			lights.pointLights.push_back(pLights.get(id)._Myfirst._Val);
		}
		lightStateSyncher.Synchronize();
	}
	void RenderingSystem::ViewProjectionUpdate(Components::Camera& camera)
	{
		// shader.Use();
		// shader.SetMat4("VP", camera.viewProjection);
		// shader.SetVec4("colorDefault", triangleColor);
		// shader.SetVec4("colorSelected", glm::vec4(selectedColor, triAlpha));

		shaders.pointShader->Use();
		shaders.pointShader->SetMat4("VP", camera.viewProjection);
		shaders.pointShader->SetFloat("aa", pointAA);
		shaders.pointShader->SetFloat("size", pointInternalSize);
		shaders.pointShader->SetVec4("colorDefault", pointColor);
		shaders.pointShader->SetVec4("colorSelected", glm::vec4(selectedColor, pointAlpha));

		shaders.lineShader->Use();
		shaders.lineShader->SetMat4("VP", camera.viewProjection);
		shaders.lineShader->SetFloat("width", lineWidth);
		shaders.lineShader->SetVec4("colorDefault", lineColor);
		shaders.lineShader->SetVec4("colorSelected", glm::vec4(selectedColor, lineAlpha));

		//lightMan.dShadowMapShader->SetMat4("VP", vp);
		shaders.gPassShader->Use();
		shaders.gPassShader->SetMat4("VP", camera.viewProjection);

		shaders.lPassShader->Use();
		shaders.lPassShader->SetVec3("viewPos", camera.position);

		shaders.skyboxShader->Use();
		auto vp = camera.projection * glm::mat4(glm::mat3(camera.view));
		shaders.skyboxShader->SetMat4("VP", vp);
	}
	void RenderingSystem::SyncComponents(entt::registry& reg)
	{
		using namespace Components;
		for (auto [id, pl, tr] : reg.view<PointLight, Transform>().each())
		{
			pl.position = tr.position;
		}
		for (auto [id, dr, tr] : reg.view<Drawable, Transform>().each())
		{
			dr.model.SetModelMatrix(tr.GetModelMatrix());
		}
	}
	void RenderingSystem::RenderScene(entt::registry& reg)
	{
		deferredPipeline.GeometryPass(ModelIterator(reg));
		deferredPipeline.LightPass();
	}
	void RenderingSystem::Update(entt::registry& registry, Components::Camera& camera)
	{
		SyncComponents(registry);
		ViewProjectionUpdate(camera);
		UpdateLights(registry);
		RenderScene(registry);

		Renderer::Framebuffer::BindDefault();
	}
}