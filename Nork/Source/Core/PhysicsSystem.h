#pragma once

#include "Modules/Physics/Pipeline/Pipeline.h"
#include "Components/Common.h"

namespace Nork {

	class PhysicsSystem
	{
	public:
		void Download(entt::registry& reg);
		void Upload(entt::registry& reg, bool updatePoliesForPhysics);
		void Update(entt::registry& reg);

		float physicsSpeed = 1.0f;

		Physics::Pipeline pipeline;
		Physics::World& pWorld = pipeline.world;
	};
}

