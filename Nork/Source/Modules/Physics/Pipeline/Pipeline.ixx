export module Nork.Physics:Pipeline;

export import :World;
export import :AABB;
export import :GJK;
export import :SAT;
export import :SAP;
export import :CollisionDetection;

export namespace Nork::Physics
{
	class Pipeline
	{

	public:
		Pipeline();

		std::vector<Collider> colls;
		CollisionDetection* collisionDetector;

		bool updateVelocities = true, updateRotation = true;
		bool detectCollisions = true, handleCollisions = true;
		bool genContactPoints = true;

		float g = 10;
		float coefficient = 0.1f;

		std::vector<glm::vec3> contactPoints;

		void Update(float delta);

		void GenContactPoints();
		void ResolveCollisions(float delta);
		void VelocityUpdate(float delta);
		void RotationUpdate(float delta);

		void SetColliders(std::span<Collider> colls);
		void SetModels(std::span<glm::vec3> translate, std::span<glm::quat> quaternions);

		World world;
	};
}