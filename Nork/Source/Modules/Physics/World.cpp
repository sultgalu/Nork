#include "World.h"
#include "Algorithms/AABB.h"

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

	Object::Object(const Collider& collider, const glm::vec3& size, const KinematicData& kinem)
		: localColl(collider), collider(collider), kinem(kinem)
	{
		UpdateInertia();
		SetColliderSize(size);
		localColl.Update(); 
		UpdateCollider();
	}
	float Object::CalcInertia()
	{
		AABB aabb(localColl.verts);
		float sum = 0;
		for (size_t i = 0; i < 3; i++)
			sum += aabb.max[i] - aabb.min[i];
		sum = glm::pow(sum / 3, 2) / 6;
		return sum * kinem.mass;
	}
	void Object::SetColliderSize(const glm::vec3& size)
	{
		if (size == this->size)
			return;

		auto scale = size / this->size;
		this->size = size;
		
		for (size_t j = 0; j < localColl.verts.size(); j++)
		{
			localColl.verts[j] *= scale;
		}
	}
	void Object::UpdateCollider()
	{
		if (!collider.isActive)
			return;

		collider.center = localColl.center + kinem.position;

		glm::mat4 rotation = glm::mat4_cast(kinem.quaternion);

		for (size_t j = 0; j < collider.verts.size(); j++)
		{
			// make it rotate around it's center, not the (0;0;0) coordinate
			collider.verts[j] = rotation * glm::vec4(localColl.verts[j] - localColl.center, 1);
			collider.verts[j] += kinem.position;
		}
		for (size_t j = 0; j < collider.faces.size(); j++)
		{
			collider.faces[j].norm = rotation * glm::vec4(localColl.faces[j].norm, 1);
		}
	}
}