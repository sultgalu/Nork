#include "RenderingSystem.h"
#include "Modules/Renderer/Objects/Framebuffer/GeometryFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Framebuffer/LightFramebufferBuilder.h"
#include "Modules/Renderer/Objects/Shader/ShaderBuilder.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Objects/VertexArray/VertexArrayBuilder.h"
#include "Modules/Renderer/Pipeline/Stages/BloomStage.h"
#include "Modules/Renderer/Pipeline/Stages/PostProcessStage.h"
#include "Modules/Renderer/Pipeline/Stages/SkyStage.h"
#include "Modules/Renderer/Pipeline/Stages/SkyboxStage.h"
#include "Modules/Renderer/Pipeline/Stages/ShadowMapStage.h"

namespace Nork {
	void RenderingSystem::UpdateDrawCommands()
	{
		std::vector<Renderer::Object> deferredObjects;
		std::vector<Renderer::Object> shadowedObjects;
		auto group = registry.group<Components::Drawable>(entt::get<Components::Transform>);
		deferredObjects.reserve(group.size());
		shadowedObjects.reserve(group.size());
		auto add = [&](const Components::Drawable& dr, std::vector<Renderer::Object>& to)
		{
			for (size_t i = 0; i < dr.GetModel()->meshes.size(); i++)
			{
				auto& mesh = dr.GetModel()->meshes[i];
				to.push_back(Renderer::Object{ .mesh = mesh.mesh, .material = mesh.material, .modelMatrix = dr.transforms[i] });
			}
		};
		for (auto [id, dr, tr] : group.each())
		{
			if (true) // eg.: its material has a deferredShader
			{
				add(dr, deferredObjects);
			}
			if (true) // eg.: not opaque
			{
				add(dr, shadowedObjects);
			}
		}
		deferredDrawCommand = Renderer::DrawObjectsCommand(world.vao, deferredObjects);
		shadowMapDrawCommand = Renderer::DrawObjectsCommand(world.vao, shadowedObjects);
	}
	void RenderingSystem::OnDrawableUpdated(entt::registry& reg, entt::entity id)
	{
		shouldUpdateDrawCommands = true;
	}
	void RenderingSystem::OnDrawableRemoved(entt::registry& reg, entt::entity id)
	{
		shouldUpdateDrawCommands = true;
	}
	void RenderingSystem::OnDrawableAdded(entt::registry& reg, entt::entity id)
	{
		auto& dr = reg.get<Components::Drawable>(id);
		auto tr = reg.try_get<Components::Transform>(id);
		dr.sharedTransform = world.AddTransform();
		if (tr)
			*dr.sharedTransform = tr->RecalcModelMatrix();
		if (dr.GetModel() == nullptr || dr.GetModel()->meshes.empty())
			dr.SetModel(ResourceUtils::GetTemplate(ModelTemplate::Cube));
		shouldUpdateDrawCommands = true;
	}
	void RenderingSystem::UpdateDirLightShadows()
	{
		Renderer::LightShadowIndices idxs;
		shadowMapProvider.dShadMaps.clear();

		auto ls = registry.view<Components::DirShadowMap>();
		auto l = registry.view<Components::DirLight>(entt::exclude<Components::DirShadowMap>);
		idxs.lightAndShadows.reserve(ls.size());
		idxs.lights.reserve(l.size_hint());
		for (auto [id, shadMap] : ls.each())
		{
			idxs.lightAndShadows.push_back({ shadMap.map.light.Index(), shadMap.map.shadow.Index() });
			shadowMapProvider.dShadMaps.push_back(shadMap.map);
		}
		for (auto [id, light] : l.each())
		{
			idxs.lights.push_back(light.light.Index());
		}
		world.DirLightIndices(idxs);
	}
	void RenderingSystem::UpdatePointLightShadows()
	{
		Renderer::LightShadowIndices idxs;
		shadowMapProvider.pShadMaps.clear();

		auto ls = registry.view<Components::PointShadowMap>();
		auto l = registry.view<Components::PointLight>(entt::exclude<Components::PointShadowMap>);
		idxs.lightAndShadows.reserve(ls.size());
		idxs.lights.reserve(l.size_hint());
		for (auto [id, shadMap] : ls.each())
		{
			idxs.lightAndShadows.push_back({ shadMap.map.light.Index(), shadMap.map.shadow.Index() });
			shadowMapProvider.pShadMaps.push_back(shadMap.map);
		}
		for (auto [id, light] : l.each())
		{
			idxs.lights.push_back(light.light.Index());
		}
		world.PointLightIndices(idxs);
	}
	void RenderingSystem::OnDLightAdded(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::DirLight>(id);
		light.light = world.AddDirLight();
		light.RecalcVP();
		shouldUpdateDirLightAndShadows = true;
	}
	void RenderingSystem::OnPLightAdded(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::PointLight>(id);
		auto& tr = reg.get_or_emplace<Components::Transform>(id);
		light.light = world.AddPointLight();
		light.SetIntensity(50);
		light.light->position = tr.Position();
		shouldUpdatePointLightAndShadows = true;
	}
	void RenderingSystem::OnDShadAdded(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::DirLight>(id);
		auto shadow = world.AddDirShadow();
		Renderer::DirShadowMap map(light.light, shadow);
		map.SetFramebuffer(4000, 4000, Renderer::TextureFormat::Depth16);
		reg.get<Components::DirShadowMap>(id).map = map;
		shouldUpdateDirLightAndShadows = true;
	}
	void RenderingSystem::OnPShadAdded(entt::registry& reg, entt::entity id)
	{
		auto& light = reg.get<Components::PointLight>(id);
		auto shadow = world.AddPointShadow();
		Renderer::PointShadowMap map(light.light, shadow);
		map.SetFramebuffer(100, Renderer::TextureFormat::Depth16);
		reg.get<Components::PointShadowMap>(id).map = map;
		shouldUpdatePointLightAndShadows = true;
	}

	void RenderingSystem::OnDShadRemoved(entt::registry& reg, entt::entity id)
	{
		shouldUpdateDirLightAndShadows = true;
	}
	void RenderingSystem::OnPShadRemoved(entt::registry& reg, entt::entity id)
	{
		shouldUpdatePointLightAndShadows = true;
	}
	void RenderingSystem::OnDLightRemoved(entt::registry& reg, entt::entity id)
	{
		reg.remove<Components::DirShadowMap>(id);
		shouldUpdateDirLightAndShadows = true;
	}
	void RenderingSystem::OnPLightRemoved(entt::registry & reg, entt::entity id)
	{
		reg.remove<Components::PointShadowMap>(id);
		shouldUpdatePointLightAndShadows = true;
	}
	RenderingSystem::RenderingSystem(entt::registry& registry)
		: registry(registry)
	{
		registry.on_update<Components::Drawable>().connect<&RenderingSystem::OnDrawableUpdated>(this);
		registry.on_destroy<Components::Drawable>().connect<&RenderingSystem::OnDrawableRemoved>(this);
		registry.on_construct<Components::Drawable>().connect<&RenderingSystem::OnDrawableAdded>(this);
		
		registry.on_construct<Components::DirShadowMap>().connect<&RenderingSystem::OnDShadAdded>(this);
		registry.on_construct<Components::PointShadowMap>().connect<&RenderingSystem::OnPShadAdded>(this);
		registry.on_destroy<Components::DirShadowMap>().connect<&RenderingSystem::OnDShadRemoved>(this);
		registry.on_destroy<Components::PointShadowMap>().connect<&RenderingSystem::OnPShadRemoved>(this);

		registry.on_construct<Components::DirLight>().connect<&RenderingSystem::OnDLightAdded>(this);
		registry.on_construct<Components::PointLight>().connect<&RenderingSystem::OnPLightAdded>(this);
		registry.on_destroy<Components::DirLight>().connect<&RenderingSystem::OnDLightRemoved>(this);
		registry.on_destroy<Components::PointLight>().connect<&RenderingSystem::OnPLightRemoved>(this);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		UpdateGlobalUniform();

		dirLightObserver.connect(registry, entt::collector.update<Components::DirLight>());
		pointLightObserver.connect(registry, entt::collector.update<Components::Transform>()
			.where<Components::PointLight>().update<Components::PointLight>());
		transformObserver.connect(registry, entt::collector.update<Components::Transform>()
			.where<Components::Drawable>().group<Components::Drawable, Components::Transform>());

		// auto sceneView = std::make_shared<SceneView>(1920, 1080);
		// sceneView->pipeline->stages.push_back(CreateStage<Renderer::SkyStage>(*sceneView->pipeline));
		// sceneView->pipeline->stages.push_back(CreateStage<Renderer::BloomStage>(*sceneView->pipeline));
		// sceneView->pipeline->stages.push_back(CreateStage<Renderer::PostProcessStage>(*sceneView->pipeline));
		//sceneViews.insert(sceneView);
	}
	void RenderingSystem::UpdateGlobalUniform()
	{
		glPointSize(globalShaderUniform.pointSize);
		shaders.pointShader->Use()
			.SetFloat("aa", globalShaderUniform.pointAA)
			.SetFloat("size", globalShaderUniform.pointInternalSize)
			.SetFloat("bias", globalShaderUniform.pointBias)
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
		for (const auto ent : dirLightObserver)
		{
			auto& dl = registry.get<Components::DirLight>(ent);
			auto ds = registry.try_get<Components::DirShadowMap>(ent);
			dl.RecalcVP();
			if (dl.sun)
			{
				float height = glm::normalize(-dl.light->direction).y;
				float redness = std::clamp(height, 0.0f, 0.2f) * 5;
				redness = 1 - redness;
				dl.light->color2 = dl.light->color - redness * glm::vec4(0.0f, 0.7f, 1.0f, 0.0f);
				
				dl.light->color.a = std::clamp(height + 0.2f, 0.0f, 0.3f) * 2.0f;
				shaders.skyShader->Use().SetVec3("lightPos", -dl.light->direction);
			}
			if (ds)
			{
				for (auto& map : shadowMapProvider.dShadMaps)
				{ // change shadow FBs in provider
					if (map.light == dl.light && map.fb != ds->map.fb)
						map.fb = ds->map.fb;
				}
			}
		}
		dirLightObserver.clear();
		for (const auto ent : pointLightObserver)
		{
			auto& pl= registry.get<Components::PointLight>(ent);
			auto ps = registry.try_get<Components::PointShadowMap>(ent);
			pl.light->position = registry.get<Components::Transform>(ent).Position();
			if (ps)
			{
				for (auto& map : shadowMapProvider.pShadMaps)
				{ // change shadow FBs in provider
					if (map.light == pl.light && map.fb != ps->map.fb)
						map.fb = ps->map.fb;
				}
			}
		}
		pointLightObserver.clear();
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
	void RenderingSystem::BeginFrame()
	{
		UpdateLights();
		for (auto& ent : transformObserver)
		{
			auto& dr = registry.get<Components::Drawable>(ent);
			const auto& modelMatrix = registry.get<Components::Transform>(ent).modelMatrix;
			*dr.sharedTransform = modelMatrix;
			for (size_t i = 0; i < dr.GetModel()->meshes.size(); i++)
			{
				auto& mesh = dr.GetModel()->meshes[i];
				if (mesh.localTransform.has_value())
					*dr.transforms[i] = modelMatrix * *mesh.localTransform;
			}
		}
		transformObserver.clear();
		if (shouldUpdateDrawCommands)
		{
			UpdateDrawCommands();
			shouldUpdateDrawCommands = false;
		}
		if (shouldUpdateDirLightAndShadows)
		{
			UpdateDirLightShadows();
			shouldUpdateDirLightAndShadows = false;
		}
		if (shouldUpdatePointLightAndShadows)
		{
			UpdatePointLightShadows();
			shouldUpdatePointLightAndShadows = false;
		}
		if (globalShaderUniform.IsChanged())
		{
			UpdateGlobalUniform();
			Logger::Info("Updating");
		}
	}
	void RenderingSystem::Update()
	{
		Timer t;
		for (auto& sceneView : sceneViews)
		{
			if (globalShaderUniform.IsChanged())
				UpdateGlobalUniform();
			ViewProjectionUpdate(*sceneView->camera);
			sceneView->pipeline->Run();
		}
		delta = t.Elapsed();
	}
	void RenderingSystem::EndFrame()
	{
		Renderer::Framebuffer::BindDefault();
	}
	void RenderingSystem::DrawToScreen(int w, int h)
	{
		std::abort(); // make a static method Framebuffer::Default(), use that as target in a pipeline
		// Renderer::Capabilities()
		// 	.Disable().DepthTest().CullFace().Blend();
		// 
		// (*sceneViews.begin()).get()->fb->Color()->Bind2D();
		// shaders.textureShader->Use()
		// 	.SetInt("tex", 0);
		// 
		// Renderer::Framebuffer::BindDefault();
		// glViewport(0, 0, w, h);
		// Renderer::DrawUtils::DrawQuad();
	}

	static std::string GetFileContent(std::string path)
	{
		std::ifstream ifs(path);
		std::stringstream ss;
		ss << ifs.rdbuf();
		return ss.str();
	}
	static Renderer::ShaderType GetTypeByString(const std::string& str)
	{
		using enum Renderer::ShaderType;
		if (str == "vertex") [[likely]]
			return Vertex;
		else if (str == "fragment") [[likely]]
			return Fragment;
		else if (str == "geometry")
			return Geometry;
		else if (str == "compute")
			return Compute;

		Logger::Error("ERR:: Unkown shader type ", str);
		std::abort(); // :(
	}
	static std::unordered_map<Renderer::ShaderType, std::string> SplitShaders(std::string content)
	{
		const char* label = "#type";
		size_t labelLen = strlen(label);

		std::unordered_map<Renderer::ShaderType, std::string> shaderSrcs;

		size_t pos = content.find(label, 0);
		Renderer::ShaderType shadType;
		do
		{ // the file must start with #type
			size_t start = pos;
			size_t eol = content.find_first_of("\r\n", pos); // idx of the end of the "#type" line

			pos += labelLen + 1; // the first idx of "vertex"
			std::string type = content.substr(pos, eol - pos); // must be "#type vertex" with whitespaces exactly like that.
			shadType = GetTypeByString(type);

			//size_t nextLinePos = s.find_first_not_of("\r\n", eol); // filtering out spaces
			pos = content.find(label, eol); // next idx starting with #type
			if (pos == std::string::npos) // no more shaders
			{
				shaderSrcs[shadType] = content.substr(eol);
				break;
			}
			shaderSrcs[shadType] = content.substr(eol, pos - eol); // more shaders ahead

		} while (true);
		return shaderSrcs;
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
		SetLightPassShader(InitShaderFromSource("Source/Shaders/lightPass.glsl"));
		gPassShader = InitShaderFromSource("Source/Shaders/gPass.glsl");
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
			.SetInt("gPosition", 0)
			.SetInt("gBaseColor", 1)
			.SetInt("gNormal", 2)
			.SetInt("gMetallicRoughness", 3);
	}

	template<> std::shared_ptr<Renderer::BloomStage> RenderingSystem::ConstructStage(Renderer::Pipeline& dest)
	{
		return std::make_shared<Renderer::BloomStage>(shaders.bloomShader, shaders.bloom3Shader, shaders.bloom2Shader, dest.FinalTexture()->GetHeight());
	}
	template<> std::shared_ptr<Renderer::DeferredStage> RenderingSystem::ConstructStage(Renderer::Pipeline& dest)
	{
		return std::make_shared<Renderer::DeferredStage>(std::dynamic_pointer_cast<Renderer::Texture2D>(dest.source->Depth()),
			shaders.gPassShader, shaders.lPassShader, &deferredDrawCommand);
	}
	template<> std::shared_ptr<Renderer::PostProcessStage> RenderingSystem::ConstructStage(Renderer::Pipeline& dest)
	{
		return std::make_shared<Renderer::PostProcessStage>(shaders.tonemapShader);
	}
	template<> std::shared_ptr<Renderer::SkyStage> RenderingSystem::ConstructStage(Renderer::Pipeline& dest)
	{
		return std::make_shared<Renderer::SkyStage>(shaders.skyShader);
	}
	template<> std::shared_ptr<Renderer::SkyboxStage> RenderingSystem::ConstructStage(Renderer::Pipeline& dest)
	{
		auto images = Renderer::LoadUtils::LoadCubemapImages("Resources/Textures/skybox", ".jpg");
		return std::make_shared<Renderer::SkyboxStage>(shaders.skyboxShader, images);
	}
	template<> std::shared_ptr<Renderer::ShadowMapStage> RenderingSystem::ConstructStage(Renderer::Pipeline& dest)
	{
		return std::make_shared<Renderer::ShadowMapStage>(shaders.dShadowShader, shaders.pShadowShader, &shadowMapDrawCommand, &shadowMapProvider);
	}
}