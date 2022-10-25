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
		auto center = glm::vec3(Physics::Center(to.verts));
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
	void PhysicsSystem::Upload(entt::registry& reg)
	{
		using namespace Components;
		uint32_t i = 0;

		auto kinView = reg.view<Transform, Kinematic>();
		kinView.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				tr.localPosition += pWorld.kinems[i].position - tr.Position();
				kin.velocity = pWorld.kinems[i].velocity;
				kin.forces = pWorld.kinems[i].forces;
				tr.localQuaternion += pWorld.kinems[i].quaternion - tr.Quaternion();
				kin.w = pWorld.kinems[i++].w;
			});
	}
	void PhysicsSystem::Download(entt::registry& reg, bool updatePoliesForPhysics)
	{
		using namespace Components;

		auto view = reg.view<Components::Transform, Kinematic>(entt::exclude_t<Collider>());
		auto collOnlyView = reg.view<Components::Transform, Collider>(entt::exclude_t<Kinematic>());
		auto collView = reg.view<Components::Transform, Kinematic, Collider>();

		pWorld.kinems.clear();

		collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Collider& coll)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.Position(), .quaternion = tr.Quaternion(),
					.velocity = kin.velocity, .w = kin.w, .mass = kin.mass,
					.isStatic = false, .forces = kin.forces,
					.torque = kin.torque, .I = kin.I });
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Collider& coll)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.Position(), .quaternion = tr.Quaternion(),
					.velocity = glm::vec3(0),.w = glm::vec3(0), .mass = 1,
					.isStatic = true, .forces = glm::vec3(0) });
			});

		view.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.Position(), .quaternion = tr.Quaternion(),
					.velocity = kin.velocity, .w = kin.w, .mass = kin.mass,
					.isStatic = false, .forces = kin.forces,
					.torque = kin.torque, .I = kin.I });
			});

		if (updatePoliesForPhysics) // bottleneck :)
		{

			pipeline.collidersLocal.clear();
			collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Collider& coll)
				{
					pipeline.collidersLocal.push_back(Convert(coll, tr.Scale()));
				});
			
			collOnlyView.each([&](entt::entity id, Transform& tr, Collider& coll)
				{
					pipeline.collidersLocal.push_back(Convert(coll, tr.Scale()));
				});

			pipeline.SetColliders();
		}

		pipeline.SetModels();

		for (size_t i = 0; i < pWorld.kinems.size(); i++)
		{
			Physics::AABB aabb(pWorld.shapes[i].verts);
			float sum = 0;
			for (size_t i = 0; i < 3; i++)
				sum += glm::pow(aabb.max[i] - aabb.min[i], 2);
			sum /= 18.0f;
			pWorld.kinems[i].I = sum * pWorld.kinems[i].mass;
		}
	}

	void PhysicsSystem::DownloadInternal()
	{
		pipeline.SetModels();
	}

	void PhysicsSystem::Update2(entt::registry& reg)
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