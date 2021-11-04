#include "pch.h"
#include "SAT.h"
#include "../Utils.h"

namespace Nork::Physics
{
	struct ClosestFeaturesState
	{
		ClosestFeaturesState()
		{
			Reset();
		}

		void Reset()
		{
			highest = -std::numeric_limits<float>::max();
			faceAndVert = std::pair(nullptr, nullptr);
			resType = 0;
		}

		uint8_t resType;
		float highest;
		union
		{
			std::pair<Face*, glm::vec3*> faceAndVert;
			std::pair<glm::vec3*, Face*> vertAndFace;
			std::pair<Edge*, Edge*>		 edgeAndEdge;
		};

		ClosestFeatures AsResult(const Shape& shape1, const Shape& shape2)
		{
			using enum ClosestFeatures::ResultType;

			glm::vec3 direction;
			if (resType == 0)
			{
				return ClosestFeatures{
					.distance = -highest,
					.direction = -shape1.FaceNormal(*faceAndVert.first),
					.resType = FaceAndVert,
					.faceAndVert = std::pair<Face&, glm::vec3&>(*faceAndVert.first, *faceAndVert.second)
				};
			}
			else if (resType == 1)
			{
				return ClosestFeatures{
					.distance = -highest,
					.direction = shape2.FaceNormal(*vertAndFace.second),
					.resType = VertAndFace,
					.vertAndFace = std::pair<glm::vec3&, Face&>(*vertAndFace.first, *vertAndFace.second)
				};
			}
			else // edge to edge
			{
				glm::vec3 edge1 = shape1.EdgeDirection(*edgeAndEdge.first);
				glm::vec3 edge2 = shape2.EdgeDirection(*edgeAndEdge.second);
				direction = glm::cross(edge2, edge1);
				glm::vec3 edgeMiddle = shape1.EdgeMiddle(*edgeAndEdge.first);
				if (glm::dot(direction, shape1.FirstVertFromEdge(*edgeAndEdge.first) - shape1.center) > 0)
					direction *= -1;

				return ClosestFeatures{
					.distance = -highest,
					.direction = glm::normalize(direction),
					.resType = EdgeAndEdge,
					.edgeAndEdge = std::pair<Edge&, Edge&>(*edgeAndEdge.first, *edgeAndEdge.second)
				};
			}
		}
	};

	static ClosestFeaturesState state;

	SAT::SAT(const Shape& shape1, const Shape& shape2)
		: shape1(shape1), shape2(shape2)
	{
	}

	bool SAT::FacePhase(const Shape& shape1, const Shape& shape2, int resType)
	{
		glm::vec3 filterDir = shape2.center - shape1.center;

		for (size_t i = 0; i < shape1.faces.size(); i++)
		{
			//if (glm::dot(filterDir, shape1.fNorm[i]) <= 0) // discard if isn't facing the other object
			//{
			//	continue;
			//}
			glm::vec3& farthestVertTowardsFace = Farthest(shape2.verts, -shape1.fNorm[i]);
			float pointDistanceFromFaceOutwards = SignedDistance(shape1.fNorm[i], shape1.SomePointOnFace(shape1.faces[i]), farthestVertTowardsFace);
			if (pointDistanceFromFaceOutwards > state.highest)
			{
				if (resType == 0)
				{
					state.faceAndVert = std::pair(&shape1.faces[i], &farthestVertTowardsFace);
				}
				else
				{
					state.vertAndFace = std::pair(&farthestVertTowardsFace, &shape1.faces[i]);
				}
				state.highest = pointDistanceFromFaceOutwards;
				state.resType = resType;

				if (pointDistanceFromFaceOutwards > 0)
					return false; // no collision, but set a separating face,
			}
		}
		return true;
	}
	bool SAT::EdgePhase()
	{
		std::vector<Edge*> filteredEdges1, filteredEdges2;

		glm::vec3 filterDir = shape2.center - shape1.center;
		for (size_t i = 0; i < shape1.edges.size(); i++)
		{
			float sd1 = SignedDistance(filterDir, shape1.center, shape1.FirstVertFromEdge(shape1.edges[i]));
			float sd2 = SignedDistance(filterDir, shape1.center, shape1.SecondVertFromEdge(shape1.edges[i]));
			if (sd1 > 0 || sd2 > 0) // at least one edge is fully in front of center plane
			{
				filteredEdges1.push_back(&shape1.edges[i]);
			}
		}

		filterDir *= -1;
		for (size_t i = 0; i < shape2.edges.size(); i++)
		{
			float sd1 = SignedDistance(filterDir, shape2.center, shape2.FirstVertFromEdge(shape2.edges[i]));
			float sd2 = SignedDistance(filterDir, shape2.center, shape2.SecondVertFromEdge(shape2.edges[i]));
			if (sd1 > 0 || sd2 > 0) // at least one edge is fully in front of center plane
			{
				filteredEdges2.push_back(&shape2.edges[i]);
			}
		}

		for (size_t i = 0; i < filteredEdges1.size(); i++)
		{
			for (size_t j = 0; j < filteredEdges2.size(); j++)
			{
				glm::vec3 edge1ToEdge2Normal = glm::cross(shape2.EdgeDirection(*filteredEdges2[j]), shape1.EdgeDirection(*filteredEdges1[i]));
				if (edge1ToEdge2Normal == glm::zero<glm::vec3>())
					continue; // calculating with 0;0;0 normal doesn't make sense
				if (SignedDistance(edge1ToEdge2Normal, shape1.FirstVertFromEdge(*filteredEdges1[i]), shape1.center) > 0)
					edge1ToEdge2Normal *= -1; // edge normal faces inwards, correct it

				glm::vec3& closest2 = Farthest(shape2.verts, -edge1ToEdge2Normal); // Get the closest point towards c1
				glm::vec3& closest1 = Farthest(shape1.verts, std::forward<glm::vec3>(edge1ToEdge2Normal)); // Get the closest point fromwards c1
				float distance = SignedDistance(edge1ToEdge2Normal, closest1, closest2);
				if (distance > state.highest)
				{
					state.highest = distance;
					state.edgeAndEdge = std::pair(filteredEdges1[i], filteredEdges2[j]);
					state.resType = 2;

					if (distance > 0)
						return false;
				}
			}
		}

		return true;
	}


	ClosestFeatures SAT::GetResult()
	{
		state.Reset();
		if (FacePhase(shape1, shape2, 0) && FacePhase(shape2, shape1, 1) && EdgePhase())
		{
			return state.AsResult(shape1, shape2);
		}
		return state.AsResult(shape1, shape2);
	}
}