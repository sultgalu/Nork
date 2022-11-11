#pragma once

#include "Modules/Physics/Pipeline/Pipeline.h"
#include "Components/Common.h"

namespace Nork {

	class PhysicsSystem
	{
	public:
		PhysicsSystem(entt::registry& reg);
		void Download();
		void Upload();
		void Update();

		float physicsSpeed = 1.0f;

		Physics::Pipeline pipeline;
		Physics::World& pWorld = pipeline.world;
		entt::registry& reg;
		entt::observer transformObserver;
	};
}

