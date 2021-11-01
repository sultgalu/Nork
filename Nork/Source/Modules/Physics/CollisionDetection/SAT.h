#pragma once

#include "../Data/World.h"

namespace Nork::Physics
{
	struct ClosestFeatures
	{
		float distance; // positive if colliding
		glm::vec3 direction; // always points from pair.first towards to pair.second. Normalized
		enum ResultType
		{
			FaceAndVert = 0, VertAndFace, EdgeAndEdge
		};

		ResultType resType;
		union
		{
			std::pair<Face&, glm::vec3&> faceAndVert;
			std::pair<glm::vec3&, Face&> vertAndFace;
			std::pair<Edge&, Edge&>		 edgeAndEdge;
		};
	};

	class SAT
	{
	public:
		SAT(const Shape& shape1, const Shape& shape2);
		ClosestFeatures GetResult();
	private:
		bool FacePhase(const Shape& useFaces, const Shape& useVerts, int resType);
		bool EdgePhase();
	private:
		 const Shape& shape1;
		 const Shape& shape2;
	};

}
