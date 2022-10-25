#pragma once

namespace Nork::Physics
{
	struct Face
	{
		glm::vec3 norm;
		uint32_t vertIdx;
	};

	using index_t = uint32_t;
	struct Edge
	{
		union
		{
			index_t first;
			index_t x;
		};
		union
		{
			index_t second;
			index_t y;
		};

		index_t& operator[](uint32_t idx)
		{
			return ((index_t*)this)[idx];
		}
	};

	struct Collider
	{
		std::vector<glm::vec4> verts;
		std::vector<Face> faces;
		std::vector<std::vector<index_t>> faceVerts;
		std::vector<Edge> edges;
	};

	enum class CollisionType : uint32_t
	{
		FaceVert, VertFace, EdgeEdge
	};

	struct CollisionResult
	{
		glm::vec3 dir;
		float depth;
		CollisionType type;
		uint16_t featureIdx1;
		uint16_t featureIdx2;
	};

	struct KinematicData
	{
		glm::vec3 position;
		glm::quat quaternion;
		glm::vec3 velocity;
		glm::vec3 w;
		float mass;
		bool isStatic = false;
		glm::vec3 forces;
		glm::vec3 torque = glm::vec3(0);
		float I;
	};
}