#pragma once

namespace Nork::Physics
{
	struct AABB
	{
		AABB(std::span<glm::vec3> verts);
		AABB() = default;
		glm::vec3 min;
		glm::vec3 max;
	};
}

