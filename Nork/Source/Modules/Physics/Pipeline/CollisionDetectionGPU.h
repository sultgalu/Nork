#pragma once

#include "CollisionDetection.h"
#include "../Data/SSBO.h"
#include "../Data/GPU.h"

namespace Nork::Physics
{
	class CollisionDetectionGPU : public CollisionDetection
	{
	public:
		CollisionDetectionGPU();

		void SetColliders(std::span<Collider> colliders);
		void UpdateTransforms(std::span<glm::vec3> translate, std::span<glm::quat> quaternions) override;
		void SetupPhase(std::span<glm::mat4> models);
		void BroadPhase() override;
		void NarrowPhase() override;
		
		static std::vector<std::pair<std::string, float>> GetDeltas();
	public:
		std::vector<CollisionResult> results;
		std::vector<CollisionResult>& GetResults() override
		{
			return results;
		}
	private:
		void ZeroAtomicCounter();
		uint32_t GetAABBResCount();

		std::array<GLuint, 3> Get_Setup_AABB_SAT_Shaders();
		void FreeShaders();
	private:
		ShaderStorageBuffer<ShapeGPU> shapes;
		ShaderStorageBuffer<glm::mat4> models;
		ShaderStorageBuffer<glm::vec4> centers;
		ShaderStorageBuffer<AABBGPU> aabbs;
		ShaderStorageBuffer<glm::vec4> verts;
		ShaderStorageBuffer<glm::vec4> vertsIn;
		ShaderStorageBuffer<Face> faces;
		ShaderStorageBuffer<Edge> edges;
		ShaderStorageBuffer<glm::uvec2> aabbRes;
		ShaderStorageBuffer<CollisionResult> satRes;

		GLuint atomicCounter;

		GLuint setupShader, aabbShader, satShader;
		uint32_t shapeCount;

		std::vector<glm::mat4> modelCache;
	};

}

