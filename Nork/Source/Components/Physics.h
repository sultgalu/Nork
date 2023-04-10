#pragma once

#include "Modules/Physics/Data/Object.h"

namespace Nork::Components {
	struct Physics
	{
		Physics() = default;
		Nork::Physics::Object& Object() const { return handle.Get(); }
		Nork::Physics::KinematicData& Kinem() const { return handle.Get().kinem; }
		std::vector<Nork::Physics::ColliderNode>& Colliders() const { return handle.Get().colliders; }
		Nork::Physics::ObjectHandle handle;
	};
}