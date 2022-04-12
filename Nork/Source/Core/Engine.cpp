#include "Engine.h"

#include "Modules/Renderer/LoadUtils.h"
#include "Core/InputState.h"
#include "App/Application.h"

namespace Nork
{
	static std::array<int, 2> c;

	Engine::Engine()
		: uploadSem(1), updateSem(1),
		scriptSystem(scene)
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
		if (c[0] % 100 == 0)
		{
			std::stringstream ss;
			for (size_t i = 0; i < c.size(); i++)
			{
				ss << (int)c[i] << ";";
			}
			Logger::Info(ss.str());
		}
		if (physicsUpdate)
		{
			c[0]++;
			//phxCalc = false;
			//while (!phxCalcDone) {} // wait for phx to finish
			//uploadSem.acquire();
			//Logger::Info("R A");
			uploadSem.acquire();
			updateSem.acquire();
			physicsSystem.Upload(scene.registry); // upload new data
			scriptSystem.Update();
			//Logger::Info("R R");
			scriptUpdated = true;
			updateSem.release();
			uploadSem.release();
			//uploadSem.release();
			// physicsSystem.Download(scene.registry);
			// physicsSystem.Update2(scene.registry);
			// physicsSystem.Upload(scene.registry);
			// scriptUpdate -> can change uploaded data
			// phxCalcDone = false;
			// phxCalc = true; // resume physics
		}
		renderingSystem.BeginFrame();
		for (size_t i = 0; i < cameras.size(); i++)
		{
			renderingSystem.Update(cameras[i], i); // draw full updated data
		}
		renderingSystem.EndFrame(); 
		// if (physicsUpdate)
		// {
		// 	physicsSystem.Download(scene.registry);
		// 	physicsSystem.Update2(scene.registry);
		// 	physicsSystem.Upload(scene.registry);
		// }

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
		scriptSystem.Update();
		// start physics thread
		physicsSystem.Download(scene.registry);
		physicsUpdate = true;
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
					c[1]++;
					updateSem.acquire(); // don't let upload happen until done with updating
					if (scriptUpdated) // if downloading 
					{
						physicsSystem.Download(scene.registry);
						scriptUpdated = false;
					}
					else
					{
						physicsSystem.DownloadInternal();
					}
					physicsSystem.Update2(scene.registry);
					updateSem.release();
					uploadSem.acquire(); // wait until any uploading is done
					uploadSem.release();
				}
			});
	}
}