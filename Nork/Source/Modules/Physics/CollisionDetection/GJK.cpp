#include "pch.h"
#include "GJK.h"

namespace Nork::Physics
{
	// Changes both simplex and direction
	bool HandleSimplex(std::vector<glm::vec3>& simplex, glm::vec3& direction)
	{
		// A is always the most recently added point.
		if (simplex.size() == 2)
		{
			glm::vec3 edgeAB = simplex[0] - simplex[1];
			glm::vec3 edgeAO = -simplex[1];
			glm::vec3 edgeABToOrigin = EdgeNormalTowards(edgeAB, edgeAO);
			direction = (edgeABToOrigin); // SET new direction
			return false;
		}
		else if (simplex.size() == 3)
		{
			glm::vec3 edgeAB = simplex[1] - simplex[2];
			glm::vec3 edgeAC = simplex[0] - simplex[2];
			glm::vec3 edgeAO = -simplex[2];
			glm::vec3 edgeABOutwards = EdgeNormalOutwards(edgeAB, edgeAC);
			glm::vec3 edgeACOutwards = EdgeNormalOutwards(edgeAC, edgeAB);
			//if (glm::dot(edgeABOutwards, edgeAO) > 0) // triangle does not contain Origin, and it is on the other side of edgeABOutwards
			//{
			//	simplex.erase(simplex.begin() + 0); // remove point from other side (C)
			//	direction = (edgeABOutwards); // we want the next point to be towards the Origin
			//	return false;
			//}
			//else if (glm::dot(edgeACOutwards, edgeAO) > 0) // triangle does not contain Origin, and it is on the other side of edgeACOutwards
			//{
			//	simplex.erase(simplex.begin() + 1); // remove point from other side (B)
			//	direction = (edgeACOutwards); // we want the next point to be towards the Origin
			//	return false;
			//}

			direction = glm::cross(edgeAB, edgeAC);
			glm::vec3 origin = glm::vec3(0);
			float sdFromOrigin = SignedDistance(direction, simplex[2], origin);
			if (sdFromOrigin < 0)
			{
				direction *= -1;
				return false;
			}
			else if (sdFromOrigin > 0)
			{
				return false;
			}
			else [[unlikely]]
			{
				return true;
			}
		}
		else if (simplex.size() == 4) [[likely]]
		{
			glm::vec3 edgeAB = simplex[2] - simplex[3];
			glm::vec3 edgeAC = simplex[1] - simplex[3];
			glm::vec3 edgeAD = simplex[0] - simplex[3];
			glm::vec3 edgeAO = -simplex[3];

			glm::vec3 ABACNormal = glm::cross(edgeAB, edgeAC);
			if (SignedDistance(ABACNormal, simplex[3], simplex[0]) > 0)
				ABACNormal *= -1;
			glm::vec3 ABADNormal = glm::cross(edgeAB, edgeAD);
			if (SignedDistance(ABADNormal, simplex[3], simplex[1]) > 0)
				ABADNormal *= -1;
			glm::vec3 ACADNormal = glm::cross(edgeAC, edgeAD);
			if (SignedDistance(ACADNormal, simplex[3], simplex[2]) > 0)
				ACADNormal *= -1;

			static auto origin = glm::vec3(0);
			glm::vec3 newDirection;
			bool foundSeparator = false;

			if (glm::dot(ABACNormal, edgeAO) > 0) // triangle does not contain Origin, and it is on the other side of edgeABOutwards
			{
				simplex.erase(simplex.begin() + 0); // remove point from other side (D)
				newDirection = (ABACNormal); // we want the next point to be towards the Origin
				foundSeparator = true;
				//return false;
			}
			else if (!foundSeparator && glm::dot(ABADNormal, edgeAO) > 0) // triangle does not contain Origin, and it is on the other side of edgeACOutwards
			{
				simplex.erase(simplex.begin() + 1); // remove point from other side (C)
				newDirection = (ABADNormal); // we want the next point to be towards the Origin
				foundSeparator = true;
				//return false;
			}
			else if (!foundSeparator && glm::dot(ACADNormal, edgeAO) > 0) // triangle does not contain Origin, and it is on the other side of edgeACOutwards
			{
				simplex.erase(simplex.begin() + 2); // remove point from other side (B)
				newDirection = (ACADNormal); // we want the next point to be towards the Origin
				foundSeparator = true;
				//return false;
			}

			if (foundSeparator)
			{
				newDirection = glm::normalize(newDirection);
				newDirection *= 1000;

				auto roundedNew = glm::round(newDirection);
				auto roundedPrev = glm::round(direction);

				if (roundedNew == roundedPrev)
				{
					return true; // we converged
				}

				// uint32_t toEliminate = Farthest(simplex, -newDirection);
				// simplex.erase(simplex.begin() + toEliminate);
				direction = newDirection;
				return false;
			}
			
			return true;
		}
	}

	// dir does not need to be a unit vector. 
	inline glm::vec3 Support(std::span<glm::vec3> verts1, std::span<glm::vec3> verts2, glm::vec3& dir)
	{
		glm::vec3& furthest1 = Farthest(verts1, std::forward<glm::vec3>(dir));
		glm::vec3& furthest2 = Farthest(verts2, -dir);
		return furthest1 - furthest2;
	}

	bool GJK::Calculate(glm::vec3 direction)
	{
		std::vector<glm::vec3> simplex = { Support(verts1, verts2, direction) }; // Get the first point of the simplex
		direction = (-simplex[0]); // next point should be towards the origin.

		while (true)
		{
			glm::vec3 nextPoint = Support(verts1, verts2, direction);
			if (glm::dot(direction, nextPoint) < 0)
				return false; // found a line separating the two shapes

			simplex.push_back(nextPoint);
			if (HandleSimplex(simplex, direction))
				return true;
		}
	}
}