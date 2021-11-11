#pragma once

namespace Nork::Physics
{
	class MD
	{
	public:
		static std::vector<glm::vec3> MinkowskiDifference(std::span<glm::vec3> verts1, std::span<glm::vec3> verts2);
	};
}

