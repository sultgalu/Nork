#include "World.h"

namespace Nork::Physics
{
	static World* world;

	World::World()
	{
		world = this;
	}

	Object& ObjectHandle::Get() const
	{
		return world->objs[world->handleObjIdxMap[*this]];
	}

	Object::Object(const std::vector<ColliderNode>& colliders, const glm::vec3& size, const KinematicData& kinem)
		: size(size), colliders(colliders), kinem(kinem)
	{
		OnLocalColliderChanged();
		UpdateGlobalColliders();
	}
	void Object::CalcInertia()
	{
		float sum = 0;
		std::vector<glm::vec3> verts;
		for (auto& collider : colliders) {
			verts.insert(verts.end(), collider.local.verts.begin(), collider.local.verts.end());
		}
		AABB aabb(verts);
		aabb.min *= size;
		aabb.max *= size;
		for (size_t i = 0; i < 3; i++)
			sum += aabb.max[i] - aabb.min[i];
		sum = glm::pow(sum / 3, 2) / 6;
		kinem.I = sum * kinem.mass;
	}
	void Object::CalcLocalCenterOfMass() {
		centerOfMassLocal = glm::vec3(0);
		volume = 0;
		for (auto& coll : colliders) {
			volume += coll.local.volume;
			centerOfMassLocal = coll.local.volume * (coll.local.center + coll.offset);
		}
		centerOfMassLocal /= volume;
		if (autoMass) {
			kinem.mass = volume * massDensity;
		}
	}
	void Object::UpdateGlobalColliders()
	{
		glm::mat4 rotation = glm::mat4_cast(kinem.quaternion);

		// centerOfMass = centerOfMassLocal;
		// centerOfMass *= size;
		centerOfMass = glm::vec3(0, 0, 0);
		for (auto& collider : colliders) {
			if (!collider.global.isActive)
				return;

			auto transformLocalToGlobal = [&](const glm::vec3& local) {
				glm::vec3 global = local + collider.offset; // local to collider
				global *= size;
				global = rotation * glm::vec4(global - centerOfMass, 1); // local to object
				global += centerOfMass;
				global += kinem.position; // local to object
				return global;
			};
			collider.global.center = transformLocalToGlobal(collider.local.center);
			for (size_t j = 0; j < collider.global.verts.size(); j++)
			{
				collider.global.verts[j] = transformLocalToGlobal(collider.local.verts[j]);
			}
			for (size_t j = 0; j < collider.global.faces.size(); j++)
			{
				collider.global.faces[j].norm = rotation * glm::vec4(collider.local.faces[j].norm, 1);
			}
			collider.aabb = AABB(collider.global.verts);
		}
		centerOfMass += kinem.position; // defer this so can rotate around it earlier
	}
	void Object::SetMass(float val)
	{
		kinem.mass = val;
		autoMass = false;
	}
	void Object::OnLocalColliderChanged()
	{
		for (auto& collider : colliders) {
			collider.local.Update();
			collider.global = collider.local;
		}
		CalcInertia();
		CalcLocalCenterOfMass();
	}
	void Object::OnMassChanged()
	{
		CalcInertia();
	}
}