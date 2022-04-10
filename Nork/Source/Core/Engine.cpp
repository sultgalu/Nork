#include "Engine.h"

#include "Modules/Renderer/LoadUtils.h"
#include "Core/InputState.h"
#include "App/Application.h"

namespace Nork
{
	Engine::Engine()
		: downloadSem(1), uploadSem(0), phxSem(1)
	{
		scene.registry.on_construct<Components::Drawable>().connect<&Engine::OnDrawableAdded>(this);
	}
	void Engine::OnDrawableAdded(entt::registry& reg, entt::entity id)
	{
		auto& dr = reg.get<Components::Drawable>(id);
		dr.model = resourceManager.GetModel("");
	
		auto* tr = reg.try_get<Components::Transform>(id);
		dr.modelMatrix = renderingSystem.drawState.modelMatrixBuffer.Add(glm::identity<glm::mat4>());
	}
	void Engine::Update()
	{
		if (physicsUpdate)
		{
			while (!phxCalcDone); // wait for phx to finish
			physicsSystem.Upload(scene.registry); // upload new data
		// scriptUpdate -> can change uploaded data
			phxCalc = true;
		}
		renderingSystem.BeginFrame();
		for (size_t i = 0; i < cameras.size(); i++)
		{
			renderingSystem.Update(cameras[i], i); // draw full updated data
		}
		renderingSystem.EndFrame(); 
		if (physicsUpdate)
		{
		}

		Nork::Window& win = Application::Get().window;
		Profiler::Clear();
		win.Refresh();
		
		// after 1 render update
		// wait for eg. 4 physics update
		// call Window.update()
		// download phyics data into registry
		// script update, which can override changes by phyics (eg. set position). Writes straight to registry
		// upload updated data to phyics
	}
	void Engine::StartPhysics()
	{
		if (physicsUpdate)
			return;
		// start physics thread
		physicsUpdate = true; 
		phxCalcDone = true;
		physicsSystem.Download(scene.registry);
		physicsThread = LaunchPhysicsThread();
	}
	void Engine::StopPhysics()
	{
		physicsUpdate = false;
		physicsThread->join();
		delete physicsThread;
	}

	void Engine::AddCamera(Components::Camera cam)
	{
		cameras.push_back(cam);
		renderingSystem.AddTarget();
	}
	
	std::thread* Engine::LaunchPhysicsThread()
	{
		return new std::thread([&]()
			{
				while (physicsUpdate)
				{
					while (!phxCalc); 
					physicsSystem.Download(scene.registry); // download changes made by scripting
					phxCalcDone = false;
					physicsSystem.Update2(scene.registry);
					for (size_t i = 0; i < 0; i++)
					{
						physicsSystem.DownloadInternal();
						physicsSystem.Update2(scene.registry);
					}
					phxCalcDone = true;
					phxCalc = false;
				}
			});
	}
}