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
				return false;
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
	inline glm::vec3 Support(std::vector<glm::vec3>& verts1, std::vector<glm::vec3>& verts2, glm::vec3& dir)
	{
		index_t furthest1 = FarthestIdx(verts1, -(-dir));
		index_t furthest2 = FarthestIdx(verts2, -dir);
		glm::vec3 res = verts1[furthest1] - verts2[furthest2];
		// verts1.erase(verts1.begin() + furthest1);
		// verts2.erase(verts2.begin() + furthest2);
		return res;
	}

	std::pair<float, glm::vec3> GJK::ClosestDepthAndDir(std::vector<glm::vec3> simplex, std::vector<glm::vec3> verts1, std::vector<glm::vec3> verts2)
	{
		std::map<float, std::vector<std::pair<std::array<uint32_t, 3>, glm::vec3>>> faces;

		auto addFace = [&](std::array<uint32_t, 3> idxs)
		{
			glm::vec3 norm = glm::normalize(glm::cross(simplex[idxs[0]] - simplex[idxs[1]], simplex[idxs[0]] - simplex[idxs[2]]));
			if (glm::dot(norm, simplex[idxs[0]]) < 0)
				norm *= -1;
			if (glm::dot(norm, norm) == 0)
			{
				Logger::Error("???"); // ?????????
				return;
			}
			float dist = SignedDistance(norm, glm::vec3(0), simplex[idxs[0]]); // OPTIMIZE: calc this first, then negate the norm if it is negative distance
			if (dist < 0)
				Logger::Error("???");
			faces[dist].push_back((dist, std::pair(idxs, norm)));
		};

		if (simplex.size() != 4) std::abort();

		addFace({ 0, 1, 2 });
		addFace({ 0, 2, 3 });
		addFace({ 0, 3, 1 });
		addFace({ 1, 2, 3 });

		while (true)
		{
			auto& closestFace = faces.begin()->second[0];
			auto closestFaceDistance = faces.begin()->first;
			auto& closestFaceNormal = closestFace.second;
			auto closestFaceIdxs = closestFace.first;

			auto closestNextPoint = Support(verts1, verts2, closestFaceNormal);
			float pDist = glm::dot(closestNextPoint, glm::normalize(closestFaceNormal));
			if (pDist < 0)
			{
				Logger::Warning("ABS(pDIST)");
			}
			if (pDist - closestFaceDistance < 0.01f)
				return std::pair(pDist, glm::normalize(-closestFaceNormal));

			std::unordered_map<uint32_t, std::unordered_set<uint32_t>> edgesToUse;
			std::map<float, std::vector<std::pair<std::array<uint32_t, 3>, glm::vec3>>> newFaces;
			for (auto& faceV : faces)
			{
				for (auto& face : faceV.second)
				{
					if (glm::dot(face.second, closestNextPoint) > 0) // this could still be misunderstood. What edges need to be removed?
					{
						auto& idxs = face.first;

						const auto edge1 = idxs[0] < idxs[1] ? std::pair(idxs[0], idxs[1]) : std::pair(idxs[1], idxs[0]);
						const auto edge2 = idxs[1] < idxs[2] ? std::pair(idxs[1], idxs[2]) : std::pair(idxs[2], idxs[1]);
						const auto edge3 = idxs[2] < idxs[0] ? std::pair(idxs[2], idxs[0]) : std::pair(idxs[0], idxs[2]);

						if (edgesToUse.contains(edge1.first) && edgesToUse[edge1.first].contains(edge1.second))
							edgesToUse[edge1.first].erase(edge1.second);
						else
							edgesToUse[edge1.first].insert(edge1.second);
					}
					else
					{
						newFaces[faceV.first].push_back(face);
					}
				}
			}
			faces = newFaces;

			uint32_t nextIdx = simplex.size();
			simplex.push_back(closestNextPoint);

			for (auto& edges : edgesToUse)
			{
				for (auto& edge : edges.second)
				{
					addFace({ edges.first, edge, nextIdx });
				}
			}
		}
	}

	std::optional<std::pair<float, glm::vec3>> GJK::Calculate(glm::vec3 direction)
	{
		std::vector<glm::vec3> verts1C;
		std::vector<glm::vec3> verts2C;
		for (size_t i = 0; i < verts1.size(); i++)
			verts1C.push_back(verts1[i]);
		for (size_t i = 0; i < verts2.size(); i++)
			verts2C.push_back(verts2[i]);
		// std::copy(verts1.begin(), verts1.end(), verts1C);
		// std::copy(verts2.begin(), verts2.end(), verts2C);
		auto verts1Copy = verts1C;
		auto verts2Copy = verts2C;

		std::vector<glm::vec3> simplex = { Support(verts1Copy, verts2Copy, direction) }; // Get the first point of the simplex
		direction = (-simplex[0]); // next point should be towards the origin.

		
		while (true)
		{
			glm::vec3 nextPoint = Support(verts1Copy, verts2Copy, direction);
			if (glm::dot(direction, nextPoint) < 0)
				return std::optional<std::pair<float, glm::vec3>>(); // found a line separating the two shapes

			simplex.push_back(nextPoint);
			if (HandleSimplex(simplex, direction))
				return ClosestDepthAndDir(simplex, verts1C, verts2C);
		}
	}
}