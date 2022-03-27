#include "RenderingSystem.h"
#include "Modules/Renderer/Objects/Framebuffer/GeometryFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Framebuffer/LightFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Shader/ShaderBuilder.h"
#include "Modules/Renderer/Pipeline/PostProcess/SkyRenderer.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"
#include "Modules/Renderer/Objects/Buffer/BufferBuilder.h"

namespace Nork {
	static std::vector<uint8_t> dShadowIndices;
	static std::vector<uint8_t> pShadowIndices;
	static constexpr auto dShadIdxSize = Renderer::Config::LightData::dirShadowsLimit;
	static constexpr auto pShadIdxSize = Renderer::Config::LightData::pointShadowsLimit;

	void RenderingSystem::OnDShadowAdded(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		shad.idx = dShadowIndices.back();
		dShadowIndices.pop_back();

		// should handle it elsewhere
		auto light = reg.try_get<Components::DirLight>(id);
		if (light != nullptr)
			shad.RecalcVP(light->GetView());
	}
	void RenderingSystem::OnDShadowRemoved(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		dShadowIndices.push_back(shad.idx);
	}
	void RenderingSystem::OnPShadAdded(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::PointShadow>(id);
		shad.idx = pShadowIndices.back();
		pShadowIndices.pop_back();
	}
	RenderingSystem::RenderingSystem(entt::registry& registry)
		: registry(registry),
		deferredPipeline(shaders.gPassShader, shaders.lPassShader, resolution.x, resolution.y),
		drawBatch(drawState.vaoWrapper.GetVertexArray())
	{
		registry.on_construct<Components::DirShadow>().connect<&RenderingSystem::OnDShadowAdded>(this);
		registry.on_destroy<Components::DirShadow>().connect<&RenderingSystem::OnDShadowRemoved>(this);
		registry.on_construct<Components::PointShadow>().connect<&RenderingSystem::OnPShadAdded>(this);

		for (int i = dShadIdxSize - 1; i > -1; i--)
		{
			dShadowIndices.push_back(i);
		}
		for (int i = pShadIdxSize - 1; i > -1; i--)
		{
			pShadowIndices.push_back(i);
		}

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
	void RenderingSystem::DrawBatchUpdate()
	{
		drawBatch.Clear();
		auto group = registry.group<Components::Drawable, Components::Transform>();

		for (auto [id, dr, tr] : registry.group<Components::Drawable, Components::Transform>().each())
		{
			drawState.modelMatrixBuffer.Update(dr.modelMatrix, tr.ModelMatrix());
			for (auto& mesh : dr.meshes)
			{
				drawBatch.AddElement(Renderer::BatchElement{
					.mesh = mesh.mesh->object,
					.material = mesh.material->object,
					.modelMatrix = dr.modelMatrix
					});
			}
		}

		drawBatch.GenerateDrawCommand();
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
	void RenderingSystem::UpdateLights()
	{
		using namespace Components;

		lightState.dirLights.clear();
		lightState.dirShadows.clear();
		for (auto [id, light, shadow] : registry.view<DirLight, DirShadow>().each())
		{
			lightState.dirLights.push_back(light);
			lightState.dirShadows.push_back(shadow);

			dirShadowMaps[shadow.idx]->Render(light, shadow, { drawBatch.GetDrawCommand() });
			dirShadowMaps[shadow.idx]->Bind(shadow);
		}
		for (auto [id, light] : registry.view<DirLight>(entt::exclude<DirShadow>).each())
		{
			lightState.dirLights.push_back(light);
		}

		lightState.pointLights.clear();
		lightState.pointShadows.clear();
		for (auto [id, light, shadow] : registry.view<PointLight, PointShadow>().each())
		{
			lightState.pointLights.push_back(light);
			lightState.pointShadows.push_back(shadow);

			pointShadowMaps[shadow.idx]->Render(light, shadow, { drawBatch.GetDrawCommand() });
			pointShadowMaps[shadow.idx]->Bind(shadow);
		}
		for (auto [id, light] : registry.view<PointLight>(entt::exclude<PointShadow>).each())
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
	void RenderingSystem::SyncComponents()
	{
		using namespace Components;
		for (auto [id, pl, tr] : registry.view<PointLight, Transform>().each())
		{
			pl.position = tr.GetPosition();
		}
	}
	void RenderingSystem::RenderScene()
	{
		deferredPipeline.GeometryPass({ drawBatch.GetDrawCommand() });
		deferredPipeline.LightPass();
		if (drawSky && skybox != nullptr)
			Renderer::SkyRenderer::RenderSkybox(*skybox, *shaders.skyboxShader);
	}
	void RenderingSystem::Update(Components::Camera& camera)
	{
		SyncComponents();
		if (globalShaderUniform.IsChanged())
		{
			UpdateGlobalUniform();
			Logger::Info("Updating");
		}
		ViewProjectionUpdate(camera);
		UpdateLights();
		DrawBatchUpdate();
		RenderScene();

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
	}

}