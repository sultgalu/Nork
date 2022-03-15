#include "RenderingSystem.h"
#include "Modules/Renderer/Objects/Framebuffer/GeometryFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Framebuffer/LightFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Shader/ShaderBuilder.h"

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

		lightState.dirLights.clear();
		lightState.dirShadows.clear();
		for (auto [id, light, shadow] : reg.view<DirLight, DirShadow>().each())
		{
			lightState.dirLights.push_back(light);
			lightState.dirShadows.push_back(shadow);

			dirShadowMaps[shadow.idx]->Render(light, shadow, ModelIterator(reg));
			dirShadowMaps[shadow.idx]->Bind(shadow);
		}
		for (auto [id, light] : reg.view<DirLight>(entt::exclude<DirShadow>).each())
		{
			lightState.dirLights.push_back(light);
		}

		lightState.pointLights.clear();
		lightState.pointShadows.clear();
		for (auto [id, light, shadow] : reg.view<PointLight, PointShadow>().each())
		{
			lightState.pointLights.push_back(light);
			lightState.pointShadows.push_back(shadow);

			pointShadowMaps[shadow.idx]->Render(light, shadow, ModelIterator(reg));
			pointShadowMaps[shadow.idx]->Bind(shadow);
		}
		for (auto [id, light] : reg.view<PointLight>(entt::exclude<PointShadow>).each())
		{
			lightState.pointLights.push_back(light);
		}
		lightState.Upload();
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
	std::shared_ptr<Renderer::Shader> Shaders::InitShaderFromSource(std::string path)
	{
		return Renderer::ShaderBuilder().Sources(SplitShaders(GetFileContent(path))).Create();
	}
	Shaders::Shaders()
	{
		SetLightPassShader(InitShaderFromSource("Source/Shaders/lightPass.shader"));
		SetGeometryPassShader(InitShaderFromSource("Source/Shaders/gPass.shader"));
		dShadowShader = InitShaderFromSource("Source/Shaders/dirShadMap.shader");
		pShadowShader = InitShaderFromSource("Source/Shaders/pointShadMap.shader");
		skyboxShader = InitShaderFromSource("Source/Shaders/skybox.shader");
		pointShader = InitShaderFromSource("Source/Shaders/point.shader");
		lineShader = InitShaderFromSource("Source/Shaders/line.shader");
		textureShader = InitShaderFromSource("Source/Shaders/texture.shader");
	}

	void Shaders::SetLightPassShader(std::shared_ptr<Renderer::Shader> shader)
	{
		lPassShader = shader;
		lPassShader->Use()
			.SetInt("gPos", 0)
			.SetInt("gDiff", 1)
			.SetInt("gNorm", 2)
			.SetInt("gSpec", 3);

		for (int i = 0; i < 5; i++)
			lPassShader->SetInt(("dirShadowMaps[" + std::to_string(i) + "]").c_str(), i + 10);
		for (int i = 0; i < 5; i++)
			lPassShader->SetInt(("pointShadowMaps[" + std::to_string(i) + "]").c_str(), i + 15);
	}

	void Shaders::SetGeometryPassShader(std::shared_ptr<Renderer::Shader> shader)
	{
		gPassShader = shader;
		using enum Renderer::TextureMapType;
		gPassShader->Use()
			.SetInt("materialTex.diffuse", (int)Diffuse)
			.SetInt("materialTex.normals", (int)Normal)
			.SetInt("materialTex.roughness", (int)Roughness)
			.SetInt("materialTex.reflect", (int)Reflection);
	}

}