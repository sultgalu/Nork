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
		static std::vector<glm::vec3> ClipFaceAgainstFaces(Shape& shape1, Face& face1, Shape& shape2, std::span<Face> faces2);
		std::vector<glm::vec3> GetAllContactPointsBF();
	private:
		std::pair<std::vector<uint32_t>, std::vector<glm::vec3>> SutherlandHodgmanClip(Face& face1, glm::vec3& faceNormal1, uint32_t point2);
	public:
		Shape& shape1;
		Shape& shape2;
	};
}

