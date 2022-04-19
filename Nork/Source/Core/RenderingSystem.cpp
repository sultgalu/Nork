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
		light.light->Update();
	}
	void RenderingSystem::OnPLightAdded(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::PointLight>(id);
		light.light = drawState.AddPointLight();
		light.SetIntensity(50);
		light.light->Update();
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
		//reg.remove<Components::PointShadowRequest>(id);
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
		auto& light = reg.get<Components::PointLight>(id);
		drawState.RemovePointLight(light.light);
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
		UpdateGlobalUniform();

		bloom.InitTextures();
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

		using namespace Renderer;
		target = FramebufferBuilder().Attachments(FramebufferAttachments().Color(
			TextureBuilder()
			.Params(TextureParams::FramebufferTex2DParams())
			.Attributes(TextureAttributes{ .width = 1920, .height = 1080, .format = TextureFormat::RGB16F })
			.Create2DEmpty(), 0))
			.Create();

		CreateCollidersVao(1);

		dirLightObserver.connect(registry, entt::collector.update<Components::DirLight>());
		pointLightObserver.connect(registry, entt::collector.update<Components::Transform>()
			.where<Components::PointLight>().update<Components::PointLight>());
	}
	void RenderingSystem::DrawBatchUpdate()
	{
		drawBatch.Clear();
		auto group = registry.group<Components::Drawable, Components::Transform>();

		for (auto [id, dr, tr] : registry.group<Components::Drawable, Components::Transform>().each())
		{
			**dr.modelMatrix = tr.modelMatrix;
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

		for (const auto ent : dirLightObserver)
		{
			auto& dl = registry.get<DirLight>(ent);
			dl.RecalcVP();
			dl.light->Update();
			if (dl.shadow)
			{
				dl.shadow->Update();
			}
			if (dl.sun)
			{
				float redness = std::clamp(-dl.light->direction.y, 0.0f, 0.2f) * 5;
				redness = 1 - redness;
				dl.light->color2 = dl.light->color - redness * glm::vec4(0.0f, 0.7f, 1.0f, 0.0f);
				
				dl.light->color.a = std::clamp(-dl.light->direction.y + 0.2f, 0.0f, 0.3f) * 2.0f;
				shaders.skyShader->Use().SetVec3("lightPos", -dl.light->direction);
			}
		}
		dirLightObserver.clear();
		for (const auto ent : pointLightObserver)
		{
			auto& pl= registry.get<PointLight>(ent);
			pl.light->position = registry.get<Transform>(ent).position;
			pl.light->Update();
			if (pl.shadow)
			{
				pl.shadow->Update();
			}
		}
		pointLightObserver.clear();

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
		shaders.skyShader->Use().SetMat4("VP", vp);
	}
	void RenderingSystem::RenderScene(Viewport& viewport)
	{
		deferredPipeline.GeometryPass({ drawBatch.GetDrawCommand() });
		deferredPipeline.LightPass();
		if (viewport.Renders(Viewport::Source::Sky))
		{
			Renderer::Capabilities()
				.Enable().DepthTest(Renderer::DepthFunc::LessOrEqual)
				.Disable().CullFace();
			shaders.skyShader->Use();
			Renderer::DrawUtils::DrawCube();
		}
	}
	void RenderingSystem::BeginFrame()
	{
		UpdateLights();
		if (globalShaderUniform.IsChanged())
		{
			UpdateGlobalUniform();
			Logger::Info("Updating");
		}
		DrawBatchUpdate();
	}
	void RenderingSystem::Update()
	{
		Timer t;
		for (auto& viewport : viewports)
		{
			if (viewport->active)
			{
				Update(*viewport);
			}
		}
		delta = t.Elapsed();
	}
	void RenderingSystem::Update(Viewport& viewport)
	{
		ViewProjectionUpdate(*viewport.camera);
		if (viewport.Renders(Viewport::Source::Deferred))
		{
			RenderScene(viewport);
			if (viewport.Renders(Viewport::Source::Bloom))
			{
				bloom.Apply(deferredPipeline.lightFb->Color(), shaders.bloomShader, shaders.bloom3Shader, shaders.bloom2Shader);
			}
			// Draw on target
		}
		target->Bind().Clear().SetViewport();
		if (viewport.Renders(Viewport::Source::Deferred))
		{
			if (viewport.Renders(Viewport::Source::Bloom))
				bloom.dest->GetAttachments().colors[0].first->Bind2D();
			else
				deferredPipeline.lightFb->Color()->Bind();

			if (viewport.Renders(Viewport::Source::Tonemap))
			{
				shaders.tonemapShader->Use().SetInt("tex", 0);
			}
			else
			{
				shaders.textureShader->Use().SetInt("tex", 0);
			}
			Renderer::DrawUtils::DrawQuad();
		}

		if (viewport.Renders(Viewport::Source::Colliders))
		{
			if (colliderVao != nullptr)
			{
				RenderColliders();
			}
		}

		if (viewport.target)
		{
			viewport.target->Bind2D();
		}
		else
		{
			target->GetAttachments().colors[0].first->Bind2D();
		}
		shaders.textureShader->Use().SetInt("tex", 0);
		viewport.fb->Bind().Clear().SetViewport();
		Renderer::DrawUtils::DrawQuad();
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
		target->GetAttachments().colors[0].first->Bind2D();
		shaders.textureShader->Use()
			.SetInt("tex", 0);

		Renderer::Framebuffer::BindDefault();
		glViewport(0, 0, w, h);
		Renderer::DrawUtils::DrawQuad();
	}
	
	void RenderingSystem::RenderColliders()
	{
		Renderer::Capabilities()
			.Enable().Blend(Renderer::BlendFunc::SrcAlpha_1MinuseSrcAlpha).CullFace()
			.Disable().DepthTest();
		shaders.colliderShader->Use();

		auto count = 0;
		auto triCount = 0;
		for (auto [id, coll] : registry.view<Components::Collider>().each())
		{
			count += coll.Points().size();
			triCount += coll.TriangleCount() * 3;
		}
		if (!colliderVao || count * sizeof(glm::vec3) > colliderVao->GetVBO()->GetSize())
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
				ptr[idx++] = tr.modelMatrix * glm::vec4(p, 1.0f);
			}
			//std::memcpy(&inds[triIdx], poly.tris.data(), poly.tris.size() * 3 * sizeof(uint32_t));
			
			for (auto& idx : coll.TriangleIndices())
			{
				inds[triIdx++] = triBase + idx;
			}
			triBase += coll.Points().size();
		}
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
	std::shared_ptr<Renderer::Shader> Shaders::InitShaderFromSource(const std::string& path)
	{
		auto shader = Renderer::ShaderBuilder().Sources(SplitShaders(GetFileContent(path))).Create();
		if (!shader)
		{
			std::abort();
		}
		shaderSources.push_back({ shader, path });
		return shader;
	}
	std::shared_ptr<Renderer::Shader> Shaders::RecompileShader(std::shared_ptr<Renderer::Shader> shader)
	{
		for (auto& [shad, source] : shaderSources)
		{
			if (shad == shader)
			{
				auto& shaderRef = FindShader(shader);
				if (RecompileShader(shaderRef, source))
				{
					shad = shaderRef;
					return shaderRef;
				}
				return nullptr;
			}
		}
		return nullptr;
	}
	bool Shaders::RecompileShader(std::shared_ptr<Renderer::Shader>& shader, const std::string& src)
	{
		auto newShad = Renderer::ShaderBuilder().Sources(SplitShaders(GetFileContent(src))).Create();
		if (newShad)
		{
			shader = newShad;
			return true;
		}
		return false;
	}
	std::shared_ptr<Renderer::Shader>& Shaders::FindShader(std::shared_ptr<Renderer::Shader> s)
	{
		if (s == gPassShader) return gPassShader;
		if (s == dShadowShader) return dShadowShader;
		if (s == pShadowShader) return pShadowShader;
		if (s == skyboxShader) return skyboxShader;
		if (s == skyShader) return skyShader;
		if (s == pointShader) return pointShader;
		if (s == lineShader) return lineShader;
		if (s == textureShader) return textureShader;
		if (s == colliderShader) return colliderShader;
		if (s == bloomShader) return bloomShader;
		if (s == bloom2Shader) return bloom2Shader;
		if (s == bloom3Shader) return bloom3Shader;
		if (s == tonemapShader) return tonemapShader;
	}
	Shaders::Shaders()
	{
		SetLightPassShader(InitShaderFromSource("Source/Shaders/lightPass.shader"));
		gPassShader = InitShaderFromSource("Source/Shaders/gPass.shader");
		skyShader = InitShaderFromSource("Source/Shaders/sky.shader");
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
		tonemapShader = InitShaderFromSource("Source/Shaders/tonemap.shader");
	}
	void Shaders::SetLightPassShader(std::shared_ptr<Renderer::Shader> shader)
	{
		lPassShader = shader;
		lPassShader->Use()
			.SetInt("gPos", 0)
			.SetInt("gDiff", 1)
			.SetInt("gNorm", 2)
			.SetInt("gSpec", 3);
	}

	Viewport::Viewport(std::shared_ptr<Components::Camera> camera)
		:camera(camera)
	{
		using namespace Renderer;
		fb = FramebufferBuilder().Attachments(FramebufferAttachments().Color(
			TextureBuilder()
			.Params(TextureParams::FramebufferTex2DParams())
			.Attributes(TextureAttributes{ .width = 1920, .height = 1080, .format = TextureFormat::RGB16F })
			.Create2DEmpty(), 0))
			.Create();
	}
}