// module Engine;

#include "Engine.h"

#include "Core/InputState.h"
import Nork.Renderer;
import Application;

namespace Nork
{
	static std::array<int, 2> c;

	Engine::Engine()
		: uploadSem(1), updateSem(1),
		scriptSystem(scene)
	{
		scene.registry.on_construct<Components::Drawable>().connect<&Engine::OnDrawableAdded>(this);
		transformObserver.connect(scene.registry, entt::collector.update<Components::Transform>());
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
		// if (c[0] % 100 == 0)
		// {
		// 	std::stringstream ss;
		// 	for (size_t i = 0; i < c.size(); i++)
		// 	{
		// 		ss << (int)c[i] << ";";
		// 	}
		// 	Logger::Info(ss.str());
		// }
		if (physicsUpdate)
		{
			c[0]++;
			uploadSem.acquire();
			updateSem.acquire();

			physicsSystem.Upload(scene.registry); // upload new data
			UpdateGlobalTransforms();

			if (scriptUpdate)
			{
				scriptSystem.Update();
				UpdateGlobalTransforms();
			}
			scriptUpdated = true;

			updateSem.release();
			uploadSem.release();
		}
		else
		{
			UpdateGlobalTransforms();
		}
		renderingSystem.BeginFrame();
		renderingSystem.Update(); // draw full updated data
		renderingSystem.EndFrame(); 
		Profiler::Clear();
		window.Refresh();
	}
	void Engine::StartPhysics(bool startScript)
	{
		if (physicsUpdate)
			return;
		if (startScript)
		{
			scriptUpdate = true;
			scriptSystem.Update();
		}
		// start physics thread
		physicsSystem.Download(scene.registry, true);
		physicsUpdate = true;
		physicsThread = LaunchPhysicsThread();
	}
	void Engine::StopPhysics()
	{
		scriptUpdate = false;
		physicsUpdate = false;
		physicsThread->join();
		delete physicsThread;
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
						physicsSystem.Download(scene.registry, true);
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

	void Engine::UpdateGlobalTransforms()
	{
		auto& reg = scene.registry;
		scene.root->ForEachDescendants([&](SceneNode& node)
			{
				auto* tr = node.GetEntity().TryGetComponent<Components::Transform>();
				if (tr)
				{
					for (auto& child : node.GetChildren())
					{
						auto* childTr = child->GetEntity().TryGetComponent<Components::Transform>();
						if (childTr)
						{
							tr->UpdateChild(*childTr);
						}
					}
					auto& parent = node.GetParent();
					if (!parent.GetEntity().HasComponent<Components::Transform>())
					{
						tr->UpdateGlobalWithoutParent();
					}
				}
			});
	}

	void Engine::UpdateTransformMatrices()
	{
		auto& reg = scene.registry;
		for (auto entity : transformObserver)
		{
			reg.get<Components::Transform>(entity).RecalcModelMatrix();
		}
	}
}