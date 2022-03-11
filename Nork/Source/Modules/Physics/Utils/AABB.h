#pragma once

#include "../Data/World.h"
#include "Modules/Renderer/Objects/Shader/Shader.h"

namespace Nork::Physics
{
	struct AABB
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	class AABBTest
	{
	public:
		AABBTest(Shape& shape1, Shape& shape2)
			: aabb1(CalcAABB(shape1)), aabb2(CalcAABB(shape2)) {}
		static std::vector<uint32_t> GetResult(World&);
		static std::vector<std::pair<uint32_t, uint32_t>> GetResult2(World&);
		static AABB GetAABB(std::span<glm::vec3> verts);
		static float GetDelta();
		static uint32_t Intersecting(AABB& aabb1, AABB& aabb2);
	private:
		static AABB CalcAABB(Shape& shape);
	private:
		AABB aabb1;
		AABB aabb2;
	};
}

