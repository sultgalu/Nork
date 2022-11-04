#pragma once

#include "Kinematic.h"
#include "Collider.h"

namespace Nork::Physics
{
	class Object
	{
	public:
		Object(const KinematicData& kinem = KinematicData())
			: kinem(kinem)
		{}

		Object(const Collider& collider, const glm::vec3& size = glm::vec3(1), const KinematicData& kinem = KinematicData())
			: localColl(collider), collider(collider), kinem(kinem)
		{
			UpdateInertia();
			SetColliderSize(size);
			UpdateCollider();
		}
		void SetColliderSize(const glm::vec3& scale);
		void UpdateCollider();
		void UpdateInertia() { this->kinem.I = CalcInertia(); }
	private:
		float CalcInertia();
	public:
		Collider localColl = Collider(); // const
		Collider collider = Collider();
		KinematicData kinem;
		glm::vec3 size = glm::vec3(1);
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