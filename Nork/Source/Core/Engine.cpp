#include "Engine.h"

#include "Modules/Renderer/LoadUtils.h"
#include "Core/InputState.h"
#include "App/Application.h"

namespace Nork
{
	Engine::Engine()
	{
		scene.registry.on_construct<Components::Drawable>().connect<&Engine::OnDrawableAdded>(this);
	}
	void Engine::OnDrawableAdded(entt::registry& reg, entt::entity id)
	{
		auto& dr = reg.get<Components::Drawable>(id);
		auto model = resourceManager.GetModel("");
		dr.meshes.push_back(Components::Mesh{ .mesh = model.front().first, .material = model.front().second });

		auto* tr = reg.try_get<Components::Transform>(id);
		dr.modelMatrix = renderingSystem.drawState.modelMatrixBuffer.Add(glm::identity<glm::mat4>());
	}
	void Engine::Launch()
	{
		Nork::Window& win = Application::Get().window;

		auto& sender = Application::Get().dispatcher.GetSender();
		while (win.IsRunning())
		{	
			sender.Send(UpdateEvent());
			sender.Send(RenderUpdateEvent());
			
			renderingSystem.Update(scene.GetMainCamera());
			if (physicsUpdate)
				physicsSystem.Update(scene.registry);

			sender.Send(RenderUpdatedEvent());
			Profiler::Clear();
			win.Refresh();

			sender.Send(UpdatedEvent());
		}
	}
}