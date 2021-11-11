#pragma once

#include "../Data/World.h"

namespace Nork::Physics
{
	/*namespace Compute
	{
		
	}
	class GPUPipeline
	{
	public:
		ShaderStorageBuffer<Compute::Shape> shapes;
		ShaderStorageBuffer<glm::mat4> models;
		ShaderStorageBuffer<glm::vec4> centers;
		ShaderStorageBuffer<Compute::AABB> aabbs;
		ShaderStorageBuffer<glm::vec4> verts;
		ShaderStorageBuffer<glm::vec4> vertsIn;
		ShaderStorageBuffer<Compute::Face> faces;
		ShaderStorageBuffer<std::array<uint32_t, 2>> edges;
		ShaderStorageBuffer<glm::uvec2> aabbRes;
		ShaderStorageBuffer<CollisionResult> satRes;

		GLuint setupShader, aabbShader, satShader;
		uint32_t shapeCount;

		std::vector<glm::uvec2>& ExecuteAABB();
		GPUPipeline(GLuint setupShader, GLuint aabbShader, GLuint satShader);
		void SetModels(std::span<glm::mat4> models);
		void SetColliders(std::span<Compute::Collider> colliders);
		
		static std::vector<std::pair<std::string, float>> GetDeltas();
		
		std::vector<CollisionResult>& Execute();
	};*/
}