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
				tr.position = pWorld.kinems[i].position;
				kin.velocity = pWorld.kinems[i].velocity;
				kin.forces = pWorld.kinems[i].forces;
				tr.quaternion = pWorld.kinems[i].quaternion;
				kin.w = pWorld.kinems[i++].w;
			});
	}
	void PhysicsSystem::Download(entt::registry& reg)
	{
		using namespace Components;

		auto view = reg.view<Components::Transform, Kinematic>(entt::exclude_t<Collider>());
		auto collOnlyView = reg.view<Components::Transform, Collider>(entt::exclude_t<Kinematic>());
		auto collView = reg.view<Components::Transform, Kinematic, Collider>();

		pWorld.kinems.clear();

		collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Collider& coll)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.position, .quaternion = tr.quaternion,
					.velocity = kin.velocity, .w = kin.w, .mass = kin.mass,
					.isStatic = false, .forces = kin.forces,
					.torque = kin.torque, .I = kin.I });
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Collider& coll)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.position, .quaternion = tr.quaternion,
					.velocity = glm::vec3(0),.w = glm::vec3(0), .mass = 1,
					.isStatic = true, .forces = glm::vec3(0) });
			});

		view.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.position, .quaternion = tr.quaternion,
					.velocity = kin.velocity, .w = kin.w, .mass = kin.mass,
					.isStatic = false, .forces = kin.forces,
					.torque = kin.torque, .I = kin.I });
			});

		if (updatePoliesForPhysics)
		{
			std::vector<Physics::Collider> colls;

			collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Collider& coll)
				{
					colls.push_back(Convert(coll, tr.scale));
				});

			collOnlyView.each([&](entt::entity id, Transform& tr, Collider& coll)
				{
					colls.push_back(Convert(coll, tr.scale));
				});
			pipeline.SetColliders(colls);
			updatePoliesForPhysics = false;
		}
		static std::vector<glm::vec3> translations;
		static std::vector<glm::quat> quaternions;
		translations.clear();
		translations.reserve(reg.size<Transform>());
		quaternions.clear();
		quaternions.reserve(reg.size<Transform>());

		collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Collider& poly)
			{
				translations.push_back(tr.position);
				quaternions.push_back(tr.quaternion);
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Collider& poly)
			{
				translations.push_back(tr.position);
				quaternions.push_back(tr.quaternion);
			});
		pipeline.SetModels(translations, quaternions);

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
		static std::vector<glm::vec3> translations;
		static std::vector<glm::quat> quaternions;
		translations.clear();
		translations.reserve(pWorld.kinems.size());
		quaternions.clear();
		quaternions.reserve(pWorld.kinems.size());

		for (size_t i = 0; i < pWorld.kinems.size(); i++)
		{
			translations.push_back(pWorld.kinems[i].position);
			quaternions.push_back(pWorld.kinems[i].quaternion);
		}
		pipeline.SetModels(translations, quaternions);
	}

	void PhysicsSystem::Update2(entt::registry& reg)
	{
		static Timer deltaTimer(-20);
		float delta = deltaTimer.ElapsedSeconds();
		deltaTimer.Restart();
		//Logger::Info(std::to_string(delta));
		if (delta > 0.2f)
			return;

		delta *= physicsSpeed;
		pipeline.Update(delta);
		//Logger::Info("Delta ", delta);
	}

}