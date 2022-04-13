#include "RenderingSystem.h"
#include "Modules/Renderer/Objects/Framebuffer/GeometryFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Framebuffer/LightFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Shader/ShaderBuilder.h"
#include "Modules/Renderer/Pipeline/PostProcess/SkyRenderer.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"
#include "Modules/Renderer/Objects/Buffer/BufferBuilder.h"
#include "Modules/Renderer/Objects/VertexArray/VertexArrayBuilder.h"

namespace Nork {
	void RenderingSystem::OnDLightAdded(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::DirLight>(id);
		light.light = drawState.AddDirLight();
	}
	void RenderingSystem::OnPLightAdded(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::PointLight>(id);
		light.light = drawState.AddPointLight();
		light.SetIntensity(50);
	}
	void RenderingSystem::OnDShadAdded(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::DirLight>(id);
		light.shadow = drawState.AddDirShadow(light.light, shaders.dShadowShader, { 4000, 4000 }, Renderer::TextureFormat::Depth16);
		//reg.remove<Components::DirShadowRequest>(id);
	}
	void RenderingSystem::OnPShadAdded(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::PointLight>(id);
		light.shadow = drawState.AddPointShadow(light.light, shaders.pShadowShader, 100, Renderer::TextureFormat::Depth16);
		reg.remove<Components::PointShadowRequest>(id);
	}

	void RenderingSystem::OnDShadRemoved(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::DirLight>(id);
		drawState.RemoveDirShadow(light.shadow);
		light.shadow = nullptr;
	}
	void RenderingSystem::OnPShadRemoved(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::PointLight>(id);
		drawState.RemovePointShadow(light.shadow);
		light.shadow = nullptr;
	}
	void RenderingSystem::OnDLightRemoved(entt::registry& reg, entt::entity id)
	{
		reg.remove<Components::DirShadowRequest>(id); // remove shadow first from UBO
		auto& light = reg.get<Components::DirLight>(id);
		drawState.RemoveDirLight(light.light);
		light.shadow = nullptr;
	}
	void RenderingSystem::OnPLightRemoved(entt::registry & reg, entt::entity id)
	{
		reg.remove<Components::PointShadowRequest>(id); // remove shadow first from UBO
		auto& light = reg.get<Components::DirLight>(id);
		drawState.RemoveDirLight(light.light);
		light.shadow = nullptr;
	}
	RenderingSystem::RenderingSystem(entt::registry& registry)
		: registry(registry),
		deferredPipeline(shaders.gPassShader, shaders.lPassShader, resolution.x, resolution.y),
		drawBatch(drawState.modelMatrixBuffer, drawState.materialBuffer, drawState.vaoWrapper)
	{
		registry.on_construct<Components::DirShadowRequest>().connect<&RenderingSystem::OnDShadAdded>(this);
		registry.on_construct<Components::PointShadowRequest>().connect<&RenderingSystem::OnPShadAdded>(this);

		registry.on_construct<Components::DirLight>().connect<&RenderingSystem::OnDLightAdded>(this);
		registry.on_construct<Components::PointLight>().connect<&RenderingSystem::OnPLightAdded>(this);

		registry.on_destroy<Components::DirShadowRequest>().connect<&RenderingSystem::OnDShadRemoved>(this);
		registry.on_destroy<Components::PointShadowRequest>().connect<&RenderingSystem::OnPShadRemoved>(this);

		registry.on_destroy<Components::DirLight>().connect<&RenderingSystem::OnDLightRemoved>(this);
		registry.on_destroy<Components::PointLight>().connect<&RenderingSystem::OnPLightRemoved>(this);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		AddTarget();
		UpdateGlobalUniform();

		bloom.InitTextures(8, 1920, 1080);
		auto image = Renderer::LoadUtils::LoadCubemapImages("Resources/Textures/skybox", ".jpg");
		std::array<void*, 6> data;
		for (size_t i = 0; i < data.size(); i++)
		{
			data[i] = image[i].data.data();
		}
		using namespace Renderer;
		skybox = TextureBuilder()
			.Attributes(TextureAttributes{ .width = image[0].width, .height = image[0].height, .format = image[0].format })
			.Params(TextureParams::CubeMapParams())
			.CreateCubeWithData(data);
	}
	void RenderingSystem::DrawBatchUpdate()
	{
		drawBatch.Clear();
		auto group = registry.group<Components::Drawable, Components::Transform>();

		for (auto [id, dr, tr] : registry.group<Components::Drawable, Components::Transform>().each())
		{
			**dr.modelMatrix = tr.ModelMatrix();
			for (auto& mesh : dr.model->meshes)
			{
				drawBatch.AddElement(Renderer::BatchElement{
					.mesh = mesh.mesh,
					.material = mesh.material,
					.modelMatrix = dr.modelMatrix
					});
			}
		}

		drawBatch.GenerateDrawCommand();
	}
	void RenderingSystem::UpdateGlobalUniform()
	{
		glPointSize(globalShaderUniform.pointSize);
		shaders.pointShader->Use()
			.SetFloat("aa", globalShaderUniform.pointAA)
			.SetFloat("size", globalShaderUniform.pointInternalSize)
			.SetVec4("colorDefault", globalShaderUniform.pointColor)
			.SetVec4("colorSelected", glm::vec4(globalShaderUniform.selectedColor, globalShaderUniform.pointAlpha));
		shaders.lineShader->Use()
			.SetFloat("width", globalShaderUniform.lineWidth)
			.SetVec4("colorDefault", globalShaderUniform.lineColor)
			.SetVec4("colorSelected", glm::vec4(globalShaderUniform.selectedColor, globalShaderUniform.lineAlpha));
		shaders.colliderShader->Use()
			.SetVec4("colorDefault", globalShaderUniform.triColor)
			.SetVec4("colorSelected", glm::vec4(globalShaderUniform.selectedColor, 1.0f));
	}
	void RenderingSystem::UpdateLights()
	{
		using namespace Components;

		for (auto [id, light] : registry.view<DirLight>().each())
		{
			if (light.shadow != nullptr)
			{
				light.shadow->shadowMap.Render(*light.light, *light.shadow, { drawBatch.GetDrawCommand() });
			}
		}
		for (auto [id, light] : registry.view<PointLight>().each())
		{
			if (light.shadow != nullptr)
			{
				light.shadow->shadowMap.Render(*light.light, *light.shadow, { drawBatch.GetDrawCommand() });
			}
		}
	}
	void RenderingSystem::ViewProjectionUpdate(Components::Camera& camera)
	{
		shaders.pointShader->Use().SetMat4("VP", camera.viewProjection);
		shaders.lineShader->Use().SetMat4("VP", camera.viewProjection);
		shaders.colliderShader->Use().SetMat4("VP", camera.viewProjection);
		
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
			pl.light->position = tr.GetPosition();
			pl.light->Update();
		}
	}
	void RenderingSystem::RenderScene()
	{
		deferredPipeline.GeometryPass({ drawBatch.GetDrawCommand() });
		deferredPipeline.LightPass();
		if (drawSky && skybox != nullptr)
			Renderer::SkyRenderer::RenderSkybox(*skybox, *shaders.skyboxShader);
	}
	void RenderingSystem::BeginFrame()
	{
		SyncComponents();
		UpdateLights();
		DrawBatchUpdate();
	}
	void RenderingSystem::Update(Components::Camera& camera, int targetIdx)
	{
		if (globalShaderUniform.IsChanged())
		{
			UpdateGlobalUniform();
			Logger::Info("Updating");
		}
		ViewProjectionUpdate(camera);
		RenderScene();
		if (useBloom && targetIdx == 0)
		{
			bloom.Apply(deferredPipeline.lightFb->Color(), shaders.bloomShader, shaders.bloom3Shader, shaders.bloom2Shader);
		}
		// Draw on target
		targetFbs[targetIdx]->Bind()
			.Clear()
			.SetViewport();
		if (useBloom && targetIdx == 0)
			bloom.dest->GetAttachments().colors[0].first->Bind2D();
		else
			deferredPipeline.lightFb->Color()->Bind();

		shaders.hdrShader->Use()
			.SetInt("tex", 0);
		Renderer::DrawUtils::DrawQuad();
		if (colliderVao != nullptr)
		{
			RenderColliders();
		}
	}
	void RenderingSystem::EndFrame()
	{
		Renderer::Framebuffer::BindDefault();
	}
	void RenderingSystem::DrawToScreen(int w, int h)
	{
		Renderer::Capabilities()
			.Disable().DepthTest().CullFace().Blend();

		//deferredPipeline.lightFb->Color()->Bind();
		targetFbs[0]->GetAttachments().colors[0].first->Bind2D();
		shaders.textureShader->Use()
			.SetInt("tex", 0);

		Renderer::Framebuffer::BindDefault();
		glViewport(0, 0, w, h);
		Renderer::DrawUtils::DrawQuad();
	}
	void RenderingSystem::AddTarget()
	{
		using namespace Renderer;
		targetFbs.push_back(FramebufferBuilder().Attachments(FramebufferAttachments().Color(
			TextureBuilder()
				.Params(TextureParams::FramebufferTex2DParams())
				.Attributes(TextureAttributes{ .width = 1920, .height = 1080, .format = TextureFormat::RGB16F })
				.Create2DEmpty(), 0))
			.Create()
		);
	}
	void RenderingSystem::RenderColliders()
	{
		auto count = 0;
		auto triCount = 0;
		for (auto [id, coll] : registry.view<Components::Collider>().each())
		{
			count += coll.Points().size();
			triCount += coll.TriangleCount() * 3;
		}
		if (count * sizeof(glm::vec3) > colliderVao->GetVBO()->GetSize())
		{
			CreateCollidersVao(count);
		}
		auto view = registry.view<Components::Collider, Components::Transform>();
		auto ptr = (glm::vec3*)colliderVao->GetVBO()->GetPersistentPtr();
		static std::vector<uint32_t> inds;
		inds.resize(triCount);

		int idx = 0;
		int triIdx = 0;
		int triBase = 0;
		for (auto [id, coll, tr] : view.each())
		{
			for (auto& p : coll.Points())
			{
				ptr[idx++] = tr.ModelMatrix() * glm::vec4(p, 1.0f);
			}
			//std::memcpy(&inds[triIdx], poly.tris.data(), poly.tris.size() * 3 * sizeof(uint32_t));
			
			for (auto& idx : coll.TriangleIndices())
			{
				inds[triIdx++] = triBase + idx;
			}
			triBase += coll.Points().size();
		}
		Renderer::Capabilities()
			.Enable().Blend(Renderer::BlendFunc::SrcAlpha_1MinuseSrcAlpha).CullFace()
			.Disable().DepthTest();
		shaders.colliderShader->Use();
		colliderVao->Bind().DrawIndexed(std::span(inds));
		//shaders.pointShader->Use();
		//colliderVao->Bind().Draw(Renderer::DrawMode::Points);
	}
	void RenderingSystem::CreateCollidersVao(size_t count)
	{
		if (count == 0)
		{
			colliderVao = nullptr;
			return;
		}
		using namespace Renderer;
		using enum BufferStorageFlags;
		auto vbo = BufferBuilder().Target(BufferTarget::Vertex)
			.Flags(WriteAccess | Persistent | Coherent).Data(nullptr, count * sizeof(glm::vec3)).Create();
		vbo->Bind().Map(BufferAccess::Write);
		colliderVao = VertexArrayBuilder().VBO(vbo).Attributes({ 3 }).Create();
	}
	void RenderingSystem::SetRenderColliders(bool opt)
	{
		if (opt)
		{
			CreateCollidersVao(registry.view<Components::Collider>().size());
		}
		else
		{
			colliderVao = nullptr;
		}
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
		colliderShader = InitShaderFromSource("Source/Shaders/position.shader");
		bloomShader = InitShaderFromSource("Source/Shaders/bloom.shader");
		bloom2Shader = InitShaderFromSource("Source/Shaders/bloom2.shader");
		bloom3Shader = InitShaderFromSource("Source/Shaders/bloom3.shader");
		hdrShader = InitShaderFromSource("Source/Shaders/hdr.shader");
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