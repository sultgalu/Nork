#include "pch.h"
#include "PhysicsSystem.h"
#include "Components/Collider.h"

namespace Nork {
	Physics::Collider Convert(const Components::Collider& from, const glm::vec3& scale)
	{
		Physics::Collider to;

		to.verts.reserve(from.Points().size());
		for (auto& point : from.Points())
		{
			to.verts.push_back(glm::vec4(point * scale, 1));
		}
		to.faceVerts.reserve(from.Faces().size());
		to.faces.reserve(from.Faces().size());
		auto center = Physics::Center(to.verts);
		for (auto& face : from.Faces())
		{
			to.faceVerts.push_back(face.points);
			auto normal = glm::normalize(glm::cross(
				from.Points()[face.points[0]] - from.Points()[face.points[1]],
				from.Points()[face.points[0]] - from.Points()[face.points[2]]
			));

			if (glm::dot(normal, from.Points()[face.points[0]] - center) < 0)
				normal *= -1; // correct normal to face against the center of the poly.
			to.faces.push_back(Physics::Face{
				.norm = normal,
				.vertIdx = face.points[0]
				});
		}
		auto actualEdges = from.Edges();
		to.edges.resize(actualEdges.size());
		std::memcpy(to.edges.data(), actualEdges.data(), actualEdges.size() * sizeof(Physics::Edge));
		return to;
	}
	void PhysicsSystem::Download(entt::registry& reg)
	{
		using namespace Components;
		uint32_t i = 0;

		reg.view<Transform, Components::Physics>()
			.each([&](entt::entity id, Transform& tr, Components::Physics& phx)
				{
					tr.localPosition += phx.handle.Get().kinem.position - tr.Position();
					tr.localQuaternion += phx.handle.Get().kinem.quaternion - tr.Quaternion();
				});
	}
	void PhysicsSystem::Upload(entt::registry& reg, bool updatePoliesForPhysics)
	{
		using namespace Components;

		reg.view<Components::Transform, Components::Physics>()
			.each([&](entt::entity id, Transform& tr, Components::Physics& phx)
				{
					phx.handle.Get().kinem.position = tr.Position();
					phx.handle.Get().kinem.quaternion = tr.Quaternion();
				});
	}

	void PhysicsSystem::Update(entt::registry& reg)
	{
		static Timer deltaTimer(-20);
		float delta = deltaTimer.ElapsedSeconds();
		deltaTimer.Restart();
		if (delta > 0.2f)
			return;

		delta *= physicsSpeed;
		pipeline.Update(delta);
	}

}