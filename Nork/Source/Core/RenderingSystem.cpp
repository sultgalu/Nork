#include "RenderingSystem.h"

namespace Nork{
	std::vector<Renderer::Model> RenderingSystem::GetModels(entt::registry& reg)
	{
		std::vector<Renderer::Model> result;

		auto view = reg.view<Components::Model, Components::Transform>();
		result.reserve(view.size_hint());

		for (auto& id : view)
		{
			auto& model = view.get(id)._Myfirst._Val;
			auto& tr = view.get(id)._Get_rest()._Myfirst._Val;

			result.push_back(Renderer::Model{ .meshes = model.meshes, .modelMatrix = tr.GetModelMatrix() });
		}

		return result;
	}
	RenderingSystem& RenderingSystem::Init()
	{
		using namespace Renderer;
		Shaders::Init();

		lightStateSyncher.Initialize();
		for (auto& fb : pShadowFramebuffers)
		{
			auto depth = TextureBuilder()
				.Params(TextureParams::CubeMapParams())
				.Attributes(TextureAttributes{ .width = 1000, .height = 1000, .format = TextureFormat::Depth16 })
				.CreateCubeEmpty();
			fb = FramebufferBuilder().Attachments(FramebufferAttachments().Depth(depth)).Create();
		}
		for (auto& fb : dShadowFramebuffers)
		{
			auto depth = TextureBuilder()
				.Params(TextureParams::FramebufferTex2DParams())
				.Attributes(TextureAttributes{ .width = 4000, .height = 4000, .format = TextureFormat::Depth16 })
				.Create2DEmpty();
			fb = FramebufferBuilder().Attachments(FramebufferAttachments().Depth(depth)).Create();
		}
		gFb = GeometryFramebufferBuilder()
			.Width(resolution.x).Height(resolution.y)
			.Depth(TextureFormat::Depth16)
			.Position(TextureFormat::RGB16F)
			.Normal(TextureFormat::RGB16F)
			.Diffuse(TextureFormat::RGB16F)
			.Specular(TextureFormat::RGBA16F)
			.Create();
		lFb = LightFramebufferBuilder()
			.DepthTexture(gFb->Depth())
			.ColorFormat(TextureFormat::RGBA16F)
			.Create();
		return *this;
	}
	void RenderingSystem::UpdateLights(entt::registry& reg)
	{
		auto dLightsWS = reg.view<Components::DirLight, Components::DirShadow>();
		auto pLightsWS = reg.view<Components::PointLight, Components::PointShadow>();
		auto dLights = reg.view<Components::DirLight>(entt::exclude<Components::DirShadow>);
		auto pLights = reg.view<Components::PointLight>(entt::exclude<Components::PointShadow>);

		auto modelView = reg.view<Components::Model, Components::Transform>();
		std::vector<Renderer::Model> models;
		for (auto& id : modelView)
		{
			models.push_back(Renderer::Model{ .meshes = modelView.get(id)._Myfirst._Val.meshes, .modelMatrix = modelView.get(id)._Get_rest()._Myfirst._Val.GetModelMatrix() });
		}
		// ---------------------------
		auto& lights = lightStateSyncher.GetLightState();
		lights.ClearAll();

		for (auto& id : dLightsWS)
		{
			const auto& light = dLightsWS.get(id)._Myfirst._Val.GetData();
			const auto& shadow = dLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();
			lights.dirLights.push_back(light);
			lights.dirShadows.push_back(shadow);

			Renderer::ShadowMapRenderer::RenderDirLightShadowMap(light, shadow, models, *dShadowFramebuffers[shadow.idx], *Shaders::dShadowShader);
			Renderer::ShadowMapRenderer::BindDirShadowMap(shadow, *dShadowFramebuffers[shadow.idx]);
		}
		for (auto& id : pLightsWS)
		{
			auto& light = pLightsWS.get(id)._Myfirst._Val.GetData();
			auto& shadow = pLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();

			lights.pointLights.push_back(light);
			lights.pointShadows.push_back(shadow);

			Renderer::ShadowMapRenderer::RenderPointLightShadowMap(light, shadow, models, *pShadowFramebuffers[shadow.idx], *Shaders::pShadowShader);
			Renderer::ShadowMapRenderer::BindPointShadowMap(shadow, *pShadowFramebuffers[shadow.idx]);
		}
		for (auto& id : dLights)
		{
			lights.dirLights.push_back(dLights.get(id)._Myfirst._Val.GetData());
		}
		for (auto& id : pLights)
		{
			lights.pointLights.push_back(pLights.get(id)._Myfirst._Val.GetData());
		}
		lightStateSyncher.Synchronize();
	}
	void RenderingSystem::ViewProjectionUpdate(Components::Camera& camera)
	{
		// shader.Use();
		// shader.SetMat4("VP", camera.viewProjection);
		// shader.SetVec4("colorDefault", triangleColor);
		// shader.SetVec4("colorSelected", glm::vec4(selectedColor, triAlpha));

		Shaders::pointShader->Use();
		Shaders::pointShader->SetMat4("VP", camera.viewProjection);
		Shaders::pointShader->SetFloat("aa", pointAA);
		Shaders::pointShader->SetFloat("size", pointInternalSize);
		Shaders::pointShader->SetVec4("colorDefault", pointColor);
		Shaders::pointShader->SetVec4("colorSelected", glm::vec4(selectedColor, pointAlpha));

		Shaders::lineShader->Use();
		Shaders::lineShader->SetMat4("VP", camera.viewProjection);
		Shaders::lineShader->SetFloat("width", lineWidth);
		Shaders::lineShader->SetVec4("colorDefault", lineColor);
		Shaders::lineShader->SetVec4("colorSelected", glm::vec4(selectedColor, lineAlpha));

		//lightMan.dShadowMapShader->SetMat4("VP", vp);
		Shaders::gPassShader->Use();
		Shaders::gPassShader->SetMat4("VP", camera.viewProjection);

		Shaders::lPassShader->Use();
		Shaders::lPassShader->SetVec3("viewPos", camera.position);

		Shaders::skyboxShader->Use();
		auto vp = camera.projection * glm::mat4(glm::mat3(camera.view));
		Shaders::skyboxShader->SetMat4("VP", vp);
	}
	void RenderingSystem::SyncComponents(entt::registry& reg)
	{
		auto pls = reg.view<Components::Transform, Components::PointLight>();

		for (auto& id : pls)
		{
			auto& tr = pls.get(id)._Myfirst._Val;
			auto& pl = pls.get(id)._Get_rest()._Myfirst._Val;

			pl.GetMutableData().position = tr.position;
		}
	}
	void RenderingSystem::RenderScene(std::span<Renderer::Model> models)
	{
		Renderer::DeferredPipeline::GeometryPass(*gFb, *Shaders::gPassShader, models);
		Renderer::DeferredPipeline::LightPass(*gFb, *lFb, *Shaders::lPassShader);
	}
	void RenderingSystem::Update(Scene& scene)
	{
		auto models = GetModels(scene.registry);

		SyncComponents(scene.registry);
		auto& cam = scene.GetMainCamera();
		ViewProjectionUpdate(cam);
		UpdateLights(scene.registry);
		RenderScene(models);

		Renderer::Framebuffer::BindDefault();
	}
}