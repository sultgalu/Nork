#include "RenderingSystem.h"
#include "Modules/Renderer/Objects/Framebuffer/GeometryFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Framebuffer/LightFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Shader/ShaderBuilder.h"
#include "Modules/Renderer/Pipeline/PostProcess/SkyRenderer.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"
#include "Modules/Renderer/Objects/Buffer/BufferBuilder.h"
#include "App/Application.h"

namespace Nork {
	std::vector<Renderer::MultiDrawCommand> MultiDrawCommand(entt::registry& reg)
	{
		static std::unordered_map<std::string, Renderer::MultiDrawCommand::InstancedDraw> instancedDraws;
		instancedDraws.clear();

		for (auto [id, dr, tr] : reg.group<Components::Drawable, Components::Transform>().each())
		{
			if (dr.meshes.empty())
				continue;
			if (!instancedDraws.contains(dr.meshes.front().mesh->id))
			{
				auto& draw = instancedDraws[dr.meshes.front().mesh->id];
				for (auto& mesh : dr.meshes)
				{
					draw.meshes.push_back({ mesh.mesh->object, mesh.material->object });
				}
			}
			instancedDraws[dr.meshes.front().mesh->id].modelMatrices.push_back(tr.ModelMatrix());
		}

		static Renderer::MultiDrawCommand command(Application::Get().engine.resourceManager.meshStorage);
		command.elements.clear();
		for (auto [id, draw] : instancedDraws)
		{
			command.elements.push_back(draw);
		}
		// auto begin = *group.raw<Components::Transform>() + group.size();
		// auto end = *group.raw<Components::Transform>() + reg.size<Components::Transform>();
		// auto curr = begin;
		// while (curr != end)
		// {
		// 	Logger::Info("tr without dr");
		// 	curr++;
		// }
		return { command };
	}
	RenderingSystem::RenderingSystem()
		: deferredPipeline(shaders.gPassShader, shaders.lPassShader, resolution.x, resolution.y)
	{
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		using namespace Renderer;
		for (auto& sm : dirShadowMaps)
		{
			sm = std::make_shared<DirShadowMap>(shaders.dShadowShader, 4000, 4000, TextureFormat::Depth16);
		}
		for (auto& sm : pointShadowMaps)
		{
			sm = std::make_shared<PointShadowMap>(shaders.pShadowShader, 100, TextureFormat::Depth16);
		}

		// auto image = Renderer::LoadUtils::LoadCubemapImages("Resources/Textures/skybox", ".jpg");
		// std::array<void*, 6> data;
		// for (size_t i = 0; i < data.size(); i++)
		// {
		// 	data[i] = image[i].data.data();
		// }
		// skybox = Renderer::TextureBuilder()
		// 	.Attributes(TextureAttributes{ .width = image[0].width, .height = image[0].height, .format = image[0].format })
		// 	.Params(TextureParams::CubeMapParams())
		// 	.CreateCubeWithData(data);

		/*materialsUbo = Renderer::BufferBuilder()
			.Target(BufferTarget::UBO)
			.Usage(BufferUsage::StaticDraw)
			.Data(nullptr, 1000 * sizeof(Renderer::ShaderDefined::Material))
			.Create();
		materialsUbo->BindBase(6);*/
	}
	void RenderingSystem::UpdateGlobalUniform()
	{
		shaders.pointShader->Use()
			.SetFloat("aa", globalShaderUniform.pointAA)
			.SetFloat("size", globalShaderUniform.pointInternalSize)
			.SetVec4("colorDefault", globalShaderUniform.pointColor)
			.SetVec4("colorSelected", glm::vec4(globalShaderUniform.selectedColor, globalShaderUniform.pointAlpha));

		shaders.lineShader->Use()
			.SetFloat("width", globalShaderUniform.lineWidth)
			.SetVec4("colorDefault", globalShaderUniform.lineColor)
			.SetVec4("colorSelected", glm::vec4(globalShaderUniform.selectedColor, globalShaderUniform.lineAlpha));
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

			dirShadowMaps[shadow.idx]->Render(light, shadow, MultiDrawCommand(reg));
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

			pointShadowMaps[shadow.idx]->Render(light, shadow, MultiDrawCommand(reg));
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
		shaders.pointShader->Use().SetMat4("VP", camera.viewProjection);
		shaders.lineShader->Use().SetMat4("VP", camera.viewProjection);
		
		shaders.gPassShader->Use().SetMat4("VP", camera.viewProjection);
		shaders.lPassShader->Use().SetVec3("viewPos", camera.position);

		auto vp = camera.projection * glm::mat4(glm::mat3(camera.view));
		shaders.skyboxShader->Use().SetMat4("VP", vp);
	}
	void RenderingSystem::SyncComponents(entt::registry& reg)
	{
		using namespace Components;
		for (auto [id, pl, tr] : reg.view<PointLight, Transform>().each())
		{
			pl.position = tr.GetPosition();
		}
	}
	void RenderingSystem::RenderScene(entt::registry& reg)
	{
		deferredPipeline.GeometryPass(MultiDrawCommand(reg));
		deferredPipeline.LightPass();
		if (drawSky && skybox != nullptr)
			Renderer::SkyRenderer::RenderSkybox(*skybox, *shaders.skyboxShader);
	}
	void RenderingSystem::Update(entt::registry& registry, Components::Camera& camera)
	{
		SyncComponents(registry);
		if (globalShaderUniform.IsChanged())
		{
			UpdateGlobalUniform();
			Logger::Info("Updating");
		}
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

		for (int i = 0; i < Renderer::Config::LightData::dirShadowsLimit; i++)
			lPassShader->SetInt(("dirShadowMaps[" + std::to_string(i) + "]").c_str(), i + Renderer::Config::LightData::dirShadowBaseIndex);
		for (int i = 0; i < Renderer::Config::LightData::pointShadowsLimit; i++)
			lPassShader->SetInt(("pointShadowMaps[" + std::to_string(i) + "]").c_str(), i + Renderer::Config::LightData::pointShadowBaseIndex);
	}

	void Shaders::SetGeometryPassShader(std::shared_ptr<Renderer::Shader> shader)
	{
		gPassShader = shader;
		// using enum Renderer::TextureMapType;
		// gPassShader->Use()
		// 	.SetInt("materialTex.diffuse", (int)Diffuse)
		// 	.SetInt("materialTex.normals", (int)Normal)
		// 	.SetInt("materialTex.roughness", (int)Roughness)
		// 	.SetInt("materialTex.reflect", (int)Reflection);
	}

}