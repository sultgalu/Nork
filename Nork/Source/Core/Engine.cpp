#include "Engine.h"

#include "Modules/Renderer/LoadUtils.h"
#include "Core/InputState.h"
#include "App/Application.h"

namespace Nork
{
	static std::vector<uint8_t> dShadowIndices;
	static std::vector<uint8_t> pShadowIndices;
	static constexpr auto dShadIdxSize = Renderer::Config::LightData::dirShadowsLimit;
	static constexpr auto pShadIdxSize = Renderer::Config::LightData::pointShadowsLimit;

	void Engine::OnDShadowAdded(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		shad.idx = dShadowIndices.back();
		dShadowIndices.pop_back();

		// should handle it elsewhere
		auto light = reg.try_get<Components::DirLight>(id);
		if (light != nullptr)
			shad.RecalcVP(light->GetView());
	}
	void Engine::OnDShadowRemoved(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		dShadowIndices.push_back(shad.idx);
	}

	static void OnPShadAdded(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::PointShadow>(id);
		shad.idx = pShadowIndices.back();
		pShadowIndices.pop_back();
	}

	void Engine::OnDrawableAdded(entt::registry& reg, entt::entity id)
	{
		auto& dr = reg.get<Components::Drawable>(id);
		dr.resource = resourceManager.GetMeshes("");
	}

	Engine::Engine(EngineConfig config)
	{
		auto& reg = scene.registry;
		reg.on_construct<Components::DirShadow>().connect<&Engine::OnDShadowAdded>(this);
		reg.on_destroy<Components::DirShadow>().connect<&Engine::OnDShadowRemoved>(this);
		reg.on_construct<Components::PointShadow>().connect<OnPShadAdded>();
		reg.on_construct<Components::Drawable>().connect<&Engine::OnDrawableAdded>(this);

		for (int i = dShadIdxSize - 1; i > -1; i--)
		{
			dShadowIndices.push_back(i);
		}
		for (int i = pShadIdxSize - 1; i > -1; i--)
		{
			pShadowIndices.push_back(i);
		}
	}

	void Engine::Launch()
	{
		Nork::Window& win = Application::Get().window;

		auto& sender = Application::Get().dispatcher.GetSender();
		while (win.IsRunning())
		{	
			sender.Send(UpdateEvent());
			sender.Send(RenderUpdateEvent());
			
			renderingSystem.Update(scene.registry, scene.GetMainCamera());
			if (physicsUpdate)
				physicsSystem.Update(scene.registry);

			sender.Send(RenderUpdatedEvent());
			Profiler::Clear();
			win.Refresh();

			sender.Send(UpdatedEvent());
		}
	}
	void Engine::ReadId(int x, int y)
	{
		/*uint32_t res;
		Renderer::Utils::Other::ReadPixels(lightFb.GetFBO(), lightFb.ColorAttForExtension(0), x, y, Renderer::Utils::Texture::Format::R32UI, &res);
		if (res != 0)
		{
			Application::Get().dispatcher.GetSender().Send(IdQueryResultEvent(x, y, res));
		}*/
	}
	Engine::~Engine()
	{
	}
}