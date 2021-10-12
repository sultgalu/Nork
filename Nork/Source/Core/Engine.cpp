#include "Engine.h"

#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Resource/DefaultResources.h"
#include "Serialization/Serializer.h"

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

			glm::mat4 rot = glm::rotate(glm::identity<glm::mat4>(), tr.rotation.z, glm::vec3(0, 0, 1));
			rot *= glm::rotate(glm::identity<glm::mat4>(), tr.rotation.x, glm::vec3(1, 0, 0));
			rot *= glm::rotate(glm::identity<glm::mat4>(), tr.rotation.y, glm::vec3(0, 1, 0)); // rotation around y-axis stays local (most used rotation usually)

			result.push_back(std::pair(model.meshes,
				glm::scale(glm::translate(glm::identity<glm::mat4>(), tr.position) * rot, tr.scale)));
		}

		return result;
	}

	Engine::Engine(EngineConfig& config)
		:window(config.width, config.height), pipeline(CreatePipelineResources())
	{
		Renderer::Resource::DefaultResources::Init();
		Serialization::Init();
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
			pipeline.DrawScene(models);

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
		auto& reg = scene.registry.GetUnderlyingMutable();
		auto dLightsWS = reg.view<Components::DirLight, Components::DirShadow>();
		auto pLightsWS = reg.view<Components::PointLight, Components::PointShadow>();
		auto dLights = reg.view<Components::DirLight>(entt::exclude<Components::DirShadow>);
		auto pLights = reg.view<Components::PointLight>(entt::exclude<Components::PointShadow>);
	
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
			auto& light = dLightsWS.get(id)._Myfirst._Val.GetData();
			auto& shadow = dLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();
			auto pair = std::pair<Data::DirLight, Data::DirShadow>(light, shadow);
			DS.push_back(pair);
		}
		for (auto& id : pLightsWS)
		{
			auto& light = pLightsWS.get(id)._Myfirst._Val.GetData();
			auto& shadow = pLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();
			PS.push_back(std::pair<Data::PointLight, Data::PointShadow>(light, shadow));
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