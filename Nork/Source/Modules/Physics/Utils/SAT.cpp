#include "pch.h"
#include "SAT.h"
#include "../Utils.h"

namespace Nork::Physics
{
	SAT::SAT(const Shape& shape1, const Shape& shape2)
		: shape1(shape1), shape2(shape2)
	{
	}

	bool SAT::FacePhase(const Shape& shape1, const Shape& shape2, CollisionType type)
	{
		glm::vec3 filterDir = shape2.center - shape1.center;

		for (size_t i = 0; i < shape1.faces.size(); i++)
		{
			//if (glm::dot(filterDir, shape1.fNorm[i]) <= 0) // discard if isn't facing the other object
			//{
			//	continue;
			//}
			index_t farthestVertTowardsFaceIdx = FarthestIdx(shape2.verts, -shape1.faces[i].norm);
			float pointDistanceFromFaceOutwards = SignedDistance(shape1.faces[i].norm, shape1.VertFromFace(i), shape2.verts[farthestVertTowardsFaceIdx]);
			if (pointDistanceFromFaceOutwards > state.depth)
			{
				state.featureIdx1 = i;
				state.featureIdx2 = farthestVertTowardsFaceIdx;
				state.depth = pointDistanceFromFaceOutwards;
				state.type = type;
				state.dir = glm::normalize(shape1.faces[i].norm);
				if (type == CollisionType::VertFace) state.dir *= -1;

				if (pointDistanceFromFaceOutwards > 0)
					return false; // no collision, but set a separating face,
			}
		}
		return true;
	}
	bool SAT::EdgePhase()
	{
		//std::vector<Edge*> filteredEdges1, filteredEdges2;
		std::vector<index_t> filteredEdges1, filteredEdges2;

		glm::vec3 filterDir = shape2.center - shape1.center;
		for (size_t i = 0; i < shape1.edges.size(); i++)
		{
			float sd1 = SignedDistance(filterDir, shape1.center, shape1.FirstVertFromEdge(shape1.edges[i]));
			float sd2 = SignedDistance(filterDir, shape1.center, shape1.SecondVertFromEdge(shape1.edges[i]));
			if (sd1 > 0 || sd2 > 0) // at least one edge is fully in front of center plane
			{
				filteredEdges1.push_back(i);
			}
		}

		filterDir *= -1;
		for (size_t i = 0; i < shape2.edges.size(); i++)
		{
			float sd1 = SignedDistance(filterDir, shape2.center, shape2.FirstVertFromEdge(shape2.edges[i]));
			float sd2 = SignedDistance(filterDir, shape2.center, shape2.SecondVertFromEdge(shape2.edges[i]));
			if (sd1 > 0 || sd2 > 0) // at least one edge is fully in front of center plane
			{
				filteredEdges2.push_back(i);
			}
		}

		for (size_t i = 0; i < filteredEdges1.size(); i++)
		{
			for (size_t j = 0; j < filteredEdges2.size(); j++)
			{
				glm::vec3 edge1ToEdge2Normal = glm::cross(shape2.EdgeDirection(shape2.edges[filteredEdges2[j]]), shape1.EdgeDirection(shape1.edges[filteredEdges1[i]]));
				if (edge1ToEdge2Normal == glm::zero<glm::vec3>())
					continue; // calculating with 0;0;0 normal doesn't make sense
				if (SignedDistance(edge1ToEdge2Normal, shape1.FirstVertFromEdge(shape1.edges[filteredEdges1[i]]), shape1.center) > 0)
					edge1ToEdge2Normal *= -1; // edge normal faces inwards, correct it

				glm::vec3& closest2 = Farthest(shape2.verts, -edge1ToEdge2Normal); // Get the closest point towards c1
				glm::vec3& closest1 = Farthest(shape1.verts, std::forward<glm::vec3>(edge1ToEdge2Normal)); // Get the closest point fromwards c1
				float distance = SignedDistance(edge1ToEdge2Normal, closest1, closest2);
				if (distance > state.depth)
				{
					state.depth = distance; 
					state.featureIdx1 = filteredEdges1[i];
					state.featureIdx2 = filteredEdges2[j];
					state.type = CollisionType::EdgeEdge;
					state.dir = glm::normalize(edge1ToEdge2Normal);

					if (distance > 0)
						return false;
				}
			}
		}

		return true;
	}


	CollisionResult SAT::GetResult()
	{
		if (FacePhase(shape1, shape2, CollisionType::FaceVert) && FacePhase(shape2, shape1, CollisionType::VertFace) && EdgePhase())
		{
			return state;
		}
		return state;
	}
}