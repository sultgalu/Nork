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

		localColl.center = glm::vec3(0);
		for (auto& vert : localColl.verts)
		{
			localColl.center += vert;
		}
		localColl.center /= localColl.verts.size();
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

	Collider Collider::Cube()
	{
		constexpr float size = 1;
		constexpr auto scale = glm::vec3(size);

		return Collider{ .verts = {
			glm::vec3(-scale.x, -scale.y, -scale.z),
			glm::vec3( scale.x, -scale.y, -scale.z),
			glm::vec3( scale.x,  scale.y, -scale.z),
			glm::vec3(-scale.x,  scale.y, -scale.z),
			glm::vec3(-scale.x, -scale.y,  scale.z),
			glm::vec3( scale.x, -scale.y,  scale.z),
			glm::vec3( scale.x,  scale.y,  scale.z),
			glm::vec3(-scale.x,  scale.y,  scale.z)
		}, .edges = {
			// Front
			Edge {.x = 0, .y = 1 },
			Edge {.x = 1, .y = 2 },
			Edge {.x = 2, .y = 3 },
			Edge {.x = 3, .y = 0 },
			// Right
			Edge {.x = 1, .y = 5 },
			Edge {.x = 5, .y = 6 },
			Edge {.x = 6, .y = 2 },
			// Left
			Edge {.x = 0, .y = 4 },
			Edge {.x = 4, .y = 7 },
			Edge {.x = 7, .y = 3 },
			// Top
			Edge {.x = 7, .y = 6 },
			// Bottom
			Edge {.x = 4, .y = 5 },
			// Back
		}, .faces = {
			Face {.norm = glm::vec3(0, 0, -1), .vertIdx = 0}, // Front
			Face {.norm = glm::vec3(1, 0, 0), .vertIdx = 1 }, // Right
			Face {.norm = glm::vec3(-1, 0, 0), .vertIdx = 4 }, // Left
			Face {.norm = glm::vec3(0, 1, 0), .vertIdx = 3 }, // Top
			Face {.norm = glm::vec3(0, -1, 0), .vertIdx = 4 }, // Bottom
			Face {.norm = glm::vec3(0, 0, 1), .vertIdx = 5 }  // Back
		}, .faceVerts = {
			{ 0, 1, 2, 3 }, // Front
			{ 1, 5, 6, 2 }, // Right
			{ 4, 0, 3, 7 }, // Left
			{ 3, 2, 6, 7 }, // Top
			{ 4, 5, 1, 0 }, // Bottom
			{ 5, 4, 7, 6 }  // Back
		}, 
			.center = glm::vec3(0),
			.isActive = true
		};
	}
}