#include "Engine.h"

#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Resource/DefaultResources.h"
#include "Serialization/BinarySerializer.h"

namespace Nork
{
	static Data::Shader CreateShaderFromPath(std::string_view path)
	{
		std::ifstream stream(path.data());
		std::stringstream buf;
		buf << stream.rdbuf();

		auto data = Data::ShaderData{ .source = buf.str() };
		auto resource = Resource::CreateShader(data);
		stream.close();
		return Data::Shader(resource);
	}

	static DeferredData CreatePipelineResources()
	{
		GLuint skyboxTex = Renderer::Utils::Texture::CreateCube("Resources/Textures/skybox", ".jpg");

		return DeferredData (DeferredData::Shaders
			{
				.gPass = CreateShaderFromPath("Source/Shaders/gPass.shader"),
				.lPass = CreateShaderFromPath("Source/Shaders/lightPass.shader"),
				.skybox = CreateShaderFromPath("Source/Shaders/skybox.shader"),
			}, skyboxTex);
	}

	static std::vector<Data::Model> GetModels(ECS::Registry& r)
	{
		std::vector<std::pair<std::vector<Data::Mesh>, glm::mat4>> result;
		auto& reg = r.GetUnderlying();

		auto view = reg.view<Components::Model, Components::Transform>();
		result.reserve(view.size_hint());

		for (auto& id : view)
		{
			auto& model = view.get(id)._Myfirst._Val;
			auto& tr = view.get(id)._Get_rest()._Myfirst._Val;

			result.push_back(std::pair(model.meshes, tr.GetModelMatrix()));
		}

		return result;
	}

	static std::vector<uint8_t> dShadowIndices;
	static std::vector<uint8_t> pShadowIndices;
	static constexpr auto dShadIdxSize = Renderer::Config::LightData::dirShadowsLimit;
	static constexpr auto pShadIdxSize = Renderer::Config::LightData::pointShadowsLimit;

	void Engine::OnDShadowAdded(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		shad.GetMutableData().idx = dShadowIndices.back();
		dShadowIndices.pop_back();

		// should handle it elsewhere
		auto light = reg.try_get<Components::DirLight>(id);
		if (light != nullptr)
			shad.RecalcVP(light->GetView());
	}
	void Engine::OnDShadowRemoved(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		dShadowIndices.push_back(shad.GetData().idx);
	}

	Engine::Engine(EngineConfig& config)
		: window(config.width, config.height), pipeline(CreatePipelineResources()),
		geometryFb(Renderer::Pipeline::GeometryFramebuffer(1920, 1080)),
		lightFb(Renderer::Pipeline::LightPassFramebuffer(geometryFb.Depth(), geometryFb.Width(), geometryFb.Height()))
	{
		Renderer::Resource::DefaultResources::Init();

		dShadowFramebuffers.reserve(dShadIdxSize);
		pShadowFramebuffers.reserve(pShadIdxSize);
		for (int i = dShadIdxSize - 1; i > -1 ; i--)
		{
			dShadowIndices.push_back(i);
			dShadowFramebuffers.push_back(ShadowFramebuffer(4000, 4000));
		}
		for (int i = pShadIdxSize - 1; i > -1 ; i--)
		{
			pShadowIndices.push_back(i);
			pShadowFramebuffers.push_back(ShadowFramebuffer(6000, 1000));
		}

		auto& reg = scene.registry.GetUnderlyingMutable();
		reg.on_construct<Components::DirShadow>().connect<&Engine::OnDShadowAdded>(this);
		reg.on_destroy<Components::DirShadow>().connect<&Engine::OnDShadowRemoved>(this);
		reg.on_update<Components::DirShadow>().connect<&Engine::OnDShadowRemoved>(this);
	}

	void Engine::Launch()
	{
		auto& sender = appEventMan.GetSender();
		while (window.IsRunning())
		{
			sender.Send(Event::Types::OnUpdate());
			sender.Send(Event::Types::OnRenderUpdate());

			auto models = GetModels(this->scene.registry);
			SyncComponents();
			ViewProjectionUpdate();
			UpdateLights();
			pipeline.DrawScene(models, lightFb, geometryFb);

			sender.Send(Event::Types::RenderUpdated());
			window.Refresh();

			sender.Send(Event::Types::Updated());
		}
	}
	Engine::~Engine()
	{
	}
	void Engine::SyncComponents()
	{
		auto& reg = scene.registry.GetUnderlyingMutable();
		auto pls = reg.view<Components::Transform, Components::PointLight>();

		for (auto& id : pls)
		{
			auto& tr = pls.get(id)._Myfirst._Val;
			auto& pl = pls.get(id)._Get_rest()._Myfirst._Val;

			pl.GetMutableData().position = tr.position;
		}
	}
	void Engine::UpdateLights()
	{
		auto& reg = scene.registry.GetUnderlying();
		auto dLightsWS = reg.view<Components::DirLight, Components::DirShadow>();
		auto pLightsWS = reg.view<Components::PointLight, Components::PointShadow>();
		auto dLights = reg.view<Components::DirLight>(entt::exclude<Components::DirShadow>);
		auto pLights = reg.view<Components::PointLight>(entt::exclude<Components::PointShadow>);
		// for shadow map drawing only
		static auto pShadowMapShader = CreateShaderFromPath("Source/Shaders/pointShadMap.shader");
		static auto dShadowMapShader = CreateShaderFromPath("Source/Shaders/dirShadMap.shader");

		auto modelView = reg.view<Components::Model, Components::Transform>();
		std::vector<std::pair<std::vector<Data::Mesh>, glm::mat4>> models;
		for (auto& id : modelView)
		{
			models.push_back(std::pair(modelView.get(id)._Myfirst._Val.meshes, modelView.get(id)._Get_rest()._Myfirst._Val.GetModelMatrix()));
		}
		// ---------------------------
	
		std::vector<std::pair<Data::DirLight, Data::DirShadow>> DS;
		std::vector<std::pair<Data::PointLight, Data::PointShadow>> PS;
		std::vector<Data::DirLight> DL;
		std::vector<Data::PointLight> PL;
		DS.reserve(dLightsWS.size_hint());
		PS.reserve(pLightsWS.size_hint());
		DL.reserve(dLights.size_hint());
		PL.reserve(pLights.size_hint());

		for (auto& id : dLightsWS)
		{
			const auto& light = dLightsWS.get(id)._Myfirst._Val.GetData();
			const auto& shadow = dLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();
			auto pair = std::pair<Data::DirLight, Data::DirShadow>(light, shadow);
			DS.push_back(pair);

			lightMan.DrawDirShadowMap(light, shadow, models, dShadowFramebuffers[shadow.idx], dShadowMapShader);
			pipeline.UseShadowMap(shadow, dShadowFramebuffers[shadow.idx]);
		}
		for (auto& id : pLightsWS)
		{
			auto& light = pLightsWS.get(id)._Myfirst._Val.GetData();
			auto& shadow = pLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();
			PS.push_back(std::pair<Data::PointLight, Data::PointShadow>(light, shadow));

			lightMan.DrawPointShadowMap(light, shadow, models, pShadowFramebuffers[shadow.idx], pShadowMapShader);
		}
		for (auto& id : dLights)
		{
			DL.push_back(dLights.get(id)._Myfirst._Val.GetData());
		}
		for (auto& id : pLights)
		{
			PL.push_back(pLights.get(id)._Myfirst._Val.GetData());
		}

		lightMan.Update(DS, PS, DL, PL);

	}
	void Engine::ViewProjectionUpdate()
	{
		auto optional = GetActiveCamera();
		if (optional.has_value())
		{
			auto& camera = *optional.value();

			//lightMan.dShadowMapShader->SetMat4("VP", vp);
			pipeline.data.shaders.gPass.Use();
			pipeline.data.shaders.gPass.SetMat4("VP", camera.viewProjection);

			pipeline.data.shaders.lPass.Use();
			pipeline.data.shaders.lPass.SetVec3("viewPos", camera.position);

			pipeline.data.shaders.skybox.Use();
			auto vp = camera.projection * glm::mat4(glm::mat3(camera.view));
			pipeline.data.shaders.skybox.SetMat4("VP", vp);
		}
		
	}
}