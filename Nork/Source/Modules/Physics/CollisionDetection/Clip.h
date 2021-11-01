#pragma once
#include "../Data/World.h"

namespace Nork::Physics
{
	class Clip
	{
	public:
		Clip(Shape& shape1, Shape& shape2)
			: shape1(shape1), shape2(shape2) { }

		bool GetResult();
	private:
		std::vector<glm::vec3> GetAllContactPointsBF();
		std::pair<std::vector<uint32_t>, std::vector<glm::vec3>> SutherlandHodgmanClip(Face& face1, glm::vec3& faceNormal1, uint32_t point2);
	public:
		Shape& shape1;
		Shape& shape2;
	};
}

