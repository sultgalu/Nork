#pragma once

#include "Modules/Physics/Data/Common.h"

namespace Nork::Components {
	struct Physics
	{
		Physics() = default;
		Nork::Physics::KinematicData& Kinem() const { return handle.Get().kinem; }
		Nork::Physics::Collider& Collider() const { return handle.Get().collider; }
		Nork::Physics::Collider& LocalCollider() const { return handle.Get().localColl; }
		Nork::Physics::ObjectHandle handle;
	};
}