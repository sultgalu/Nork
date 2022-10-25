#include "Engine.h"

#include "Modules/Renderer/LoadUtils.h"
#include "Core/InputState.h"
#include "App/Application.h"

namespace Nork
{
	Engine::Engine()
		: uploadSem(1), updateSem(1),
		scriptSystem(scene)
	{
		scene.registry.on_construct<Components::Drawable>().connect<&Engine::OnDrawableAdded>(this);
		scene.registry.on_construct<Components::Physics>().connect<&Engine::OnPhysicsAdded>(this);
		scene.registry.on_destroy<Components::Physics>().connect<&Engine::OnPhysicsRemoved>(this);
		transformObserver.connect(scene.registry, entt::collector.update<Components::Transform>());
	}
	void Engine::OnDrawableAdded(entt::registry& reg, entt::entity id)
	{
		auto& dr = reg.get<Components::Drawable>(id);
		dr.model = resourceManager.GetModel("");
	
		auto* tr = reg.try_get<Components::Transform>(id);
		dr.modelMatrix = renderingSystem.drawState.modelMatrixBuffer.Add(glm::identity<glm::mat4>());
	}
	void Engine::OnPhysicsAdded(entt::registry& reg, entt::entity id)
	{
		auto& obj = reg.get<Components::Physics>(id);

		if (reg.any_of<Components::Transform>(id))
		{
			auto& tr = reg.get<Components::Transform>(id);
			obj.handle = physicsSystem.pWorld.AddObject(Physics::Object(Physics::Collider::Cube(tr.Scale()),
				Physics::KinematicData { .position = tr.Position(), .quaternion = tr.Quaternion() }));
		}
		else
		{
			obj.handle = physicsSystem.pWorld.AddObject(Physics::Object(Physics::Collider::Cube()));
		}
	}
	void Engine::OnPhysicsRemoved(entt::registry& reg, entt::entity id)
	{
		auto& dr = reg.get<Components::Drawable>(id);
	}
	void Engine::Update()
	{
		if (physicsUpdate)
		{
			uploadSem.acquire();
			updateSem.acquire();

			physicsSystem.Download(scene.registry); // get changes from physics (writes to local Transform)
			UpdateGlobalTransforms(); // apply local changes to global Transform

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
			UpdateGlobalTransforms(); // Editor changes
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
		physicsSystem.Upload(scene.registry, true);
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
					updateSem.acquire(); // don't let upload happen until done with updating
					if (scriptUpdated) // if downloading 
					{
						physicsSystem.Upload(scene.registry, true);
						scriptUpdated = false;
					}
					physicsSystem.Update(scene.registry);
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