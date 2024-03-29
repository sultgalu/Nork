#include "Engine.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Core/InputState.h"
#include "App/Application.h"
#include "PolygonBuilder.h"
#include "Components/All.h"

namespace Nork
{
	Engine* _engine;
	static constexpr bool MULTITHREAD_PHX = true;
	Engine& Engine::Get()
	{
		return *_engine;
	}
	PhysicsSystem& PhysicsSystem::Instance()
	{
		return Engine::Get().physicsSystem;
	}
	Engine::Engine() : window(1920 * 0.8f, 1080 * 0.8f), input(window.glfwWindow),
		uploadSem(1), updateSem(1)
	{
		_engine = this;
		scene.registry.on_construct<Components::Physics>().connect<&Engine::OnPhysicsAdded>(this);
		scene.registry.on_destroy<Components::Physics>().connect<&Engine::OnPhysicsRemoved>(this);
		scriptSystem.LoadDLL("Scripts.dll");
	}
	void Engine::OnPhysicsAdded(entt::registry& reg, entt::entity id)
	{
		auto& obj = reg.get<Components::Physics>(id);

		if (reg.any_of<Components::Transform>(id))
		{
			auto& tr = reg.get<Components::Transform>(id);
			obj.handle = physicsSystem.pWorld.AddObject(Physics::Object({ Physics::ColliderNode{.local = Physics::Collider::Cube() } }, 
				tr.Scale(), Physics::KinematicData { .position = tr.Position(), .quaternion = tr.Quaternion() }));
		}
		else
		{
			obj.handle = physicsSystem.pWorld.AddObject(Physics::Object({ Physics::ColliderNode{.local = Physics::Collider::Cube() } }));
		}
	}
	void Engine::OnPhysicsRemoved(entt::registry& reg, entt::entity id)
	{
		auto& obj = reg.get<Components::Physics>(id);
		physicsSystem.pWorld.RemoveObject(obj.handle);
	}
	void Engine::Update()
	{
		if (physicsUpdate)
		{
			if (MULTITHREAD_PHX)
			{
				uploadSem.acquire();
				updateSem.acquire();
			}
			else
			{
				if (scriptUpdated)
				{
					physicsSystem.Upload();
					scriptUpdated = false;
				}
				physicsSystem.Update();
			}

			physicsSystem.Download(); // get changes from physics (writes to local Transform)
			UpdateGlobalTransforms(); // apply local changes to global Transform

			if (scriptUpdate)
			{
				scriptSystem.Update();
				UpdateGlobalTransforms();
			}
			scriptUpdated = true; // editor can also update

			if (MULTITHREAD_PHX)
			{
				updateSem.release();
				uploadSem.release();
			}
		}
		else
		{
			UpdateGlobalTransforms(); // Editor changes
		}
		renderingSystem.Update(); // draw full updated data
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
		physicsSystem.Upload();
		physicsUpdate = true;
		if (MULTITHREAD_PHX)
		{
			physicsThread = LaunchPhysicsThread();
		}
	}
	void Engine::StopPhysics()
	{
		scriptUpdate = false;
		physicsUpdate = false;
		if (MULTITHREAD_PHX)
		{
			physicsThread->join();
			delete physicsThread;
		}
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
						physicsSystem.Upload();
						scriptUpdated = false;
					}
					physicsSystem.Update();
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
					auto* parentTr = node.GetParent().GetEntity().TryGetComponent<Components::Transform>();
					if (parentTr) {
						tr->UpdateGlobalWithoutParent();
						tr->SetToDecomposed(parentTr->modelMatrix * tr->modelMatrix);
					}
					else {
						tr->UpdateGlobalWithoutParent();
					}
				}
			});
	}
}