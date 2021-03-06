#pragma once

#include "Modules/Physics/Pipeline/Pipeline.h"
#include "Components/Common.h"
#include "PolygonMesh.h"

namespace Nork {

	class PhysicsSystem
	{
	public:
		void Upload(entt::registry& reg);
		void Download(entt::registry& reg, bool updatePoliesForPhysics);
		void DownloadInternal();
		void Update2(entt::registry& reg);

		std::vector<std::pair<std::string, float>> deltas;
		bool satRes = false, gjkRes = false, clipRes = false, aabbRes = false;
		bool sat = false, gjk = false, clip = true, aabb = true;
		std::optional<std::pair<glm::vec3, std::pair<uint8_t, glm::vec3>>> collisionRes;
		float physicsSpeed = 1.0f;

		Physics::Pipeline pipeline;
		Physics::World& pWorld = pipeline.world;
	};
}

