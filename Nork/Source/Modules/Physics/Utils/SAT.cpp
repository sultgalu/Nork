#include "pch.h"
#include "SAT.h"
#include "../Utils.h"

namespace Nork::Physics
{
	SAT::SAT(const Collider& collider1, const Collider& collider2)
	{
		if (FacePhase(collider1, collider2, CollisionType::FaceVert) && FacePhase(collider2, collider1, CollisionType::VertFace) && EdgePhase(collider1, collider2))
		{
		}
	}

	bool SAT::FacePhase(const Collider& collider1, const Collider& collider2, CollisionType type)
	{
		glm::vec3 filterDir = collider2.center - collider1.center;

		for (size_t i = 0; i < collider1.faces.size(); i++)
		{
			//if (glm::dot(filterDir, collider1.fNorm[i]) <= 0) // discard if isn't facing the other object
			//{
			//	continue;
			//}
			index_t farthestVertTowardsFaceIdx = FarthestIdx(collider2.verts, -collider1.faces[i].norm);
			float pointDistanceFromFaceOutwards = SignedDistance(collider1.faces[i].norm, collider1.VertFromFace(i), collider2.verts[farthestVertTowardsFaceIdx]);
			if (pointDistanceFromFaceOutwards > state.depth)
			{
				state.featureIdx1 = i;
				state.featureIdx2 = farthestVertTowardsFaceIdx;
				state.depth = pointDistanceFromFaceOutwards;
				state.type = type;
				state.dir = glm::normalize(collider1.faces[i].norm);
				if (type == CollisionType::VertFace) state.dir *= -1;

				if (pointDistanceFromFaceOutwards > 0)
					return false; // no collision, but set a separating face,
			}
		}
		return true;
	}
	bool SAT::EdgePhase(const Collider& collider1, const Collider& collider2)
	{
		//std::vector<Edge*> filteredEdges1, filteredEdges2;
		std::vector<index_t> filteredEdges1, filteredEdges2;

		glm::vec3 filterDir = collider2.center - collider1.center;
		for (size_t i = 0; i < collider1.edges.size(); i++)
		{
			float sd1 = SignedDistance(filterDir, collider1.center, collider1.FirstVertFromEdge(collider1.edges[i]));
			float sd2 = SignedDistance(filterDir, collider1.center, collider1.SecondVertFromEdge(collider1.edges[i]));
			if (sd1 > 0 || sd2 > 0) // at least one edge is fully in front of center plane
			{
				filteredEdges1.push_back(i);
			}
		}

		filterDir *= -1;
		for (size_t i = 0; i < collider2.edges.size(); i++)
		{
			float sd1 = SignedDistance(filterDir, collider2.center, collider2.FirstVertFromEdge(collider2.edges[i]));
			float sd2 = SignedDistance(filterDir, collider2.center, collider2.SecondVertFromEdge(collider2.edges[i]));
			if (sd1 > 0 || sd2 > 0) // at least one edge is fully in front of center plane
			{
				filteredEdges2.push_back(i);
			}
		}

		for (size_t i = 0; i < filteredEdges1.size(); i++)
		{
			for (size_t j = 0; j < filteredEdges2.size(); j++)
			{
				glm::vec3 edge1ToEdge2Normal = glm::cross(collider2.EdgeDirection(collider2.edges[filteredEdges2[j]]), collider1.EdgeDirection(collider1.edges[filteredEdges1[i]]));
				if (edge1ToEdge2Normal == glm::zero<glm::vec3>())
					continue; // calculating with 0;0;0 normal doesn't make sense
				if (SignedDistance(edge1ToEdge2Normal, collider1.FirstVertFromEdge(collider1.edges[filteredEdges1[i]]), collider1.center) > 0)
					edge1ToEdge2Normal *= -1; // edge normal faces inwards, correct it

				glm::vec3& closest2 = Farthest(collider2.verts, -edge1ToEdge2Normal); // Get the closest point towards c1
				glm::vec3& closest1 = Farthest(collider1.verts, std::forward<glm::vec3>(edge1ToEdge2Normal)); // Get the closest point fromwards c1
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
		return state;
	}
}