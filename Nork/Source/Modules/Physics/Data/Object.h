#pragma once

#include "Kinematic.h"
#include "Collider.h"
#include "AABB.h"

namespace Nork::Physics
{
struct ColliderNode {
	glm::vec3 offset = glm::vec3(0);
	Collider local;
	Collider global;
	AABB aabb;
};

struct ColliderIndex {
	index_t objIdx;
	index_t collIdx;
};

class Object
{
public:
	Object(const KinematicData& kinem = KinematicData())
		: kinem(kinem)
	{}
	Object(const std::vector<ColliderNode>& colliders, const glm::vec3& size = glm::vec3(1), const KinematicData& kinem = KinematicData());
	void OnLocalColliderChanged();
	void OnMassChanged();
	void UpdateGlobalColliders();
	void SetMass(float);
private:
	void CalcInertia();
	void CalcLocalCenterOfMass();
public:
	std::vector<ColliderNode> colliders;
	KinematicData kinem;
	glm::vec3 size = glm::vec3(1);
	glm::vec3 centerOfMass;
	float volume;
	bool autoMass = true;
	float massDensity = 1.0f;
private:
	glm::vec3 centerOfMassLocal;
};

struct ObjectHandle
{
	Object& Get() const;
	bool operator==(const ObjectHandle& other) const
	{
		return handle == other.handle;
	}
public:
	uint64_t handle;
};
}