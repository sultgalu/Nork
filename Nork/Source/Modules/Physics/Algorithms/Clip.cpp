#include "Clip.h"
#include "../Utils.h"

namespace Nork::Physics 
{
	static glm::vec3 Interpolate(float sdStart, float sdEnd, const glm::vec3& start, const glm::vec3& end)
	{
		float normalDistance = sdStart - sdEnd;
		float startToFacePortion = sdStart / normalDistance;
		glm::vec3 startToEnd = end - start;
		glm::vec3 startToFace = startToEnd * startToFacePortion;
		glm::vec3 pointOnFace = start + startToFace;
		return pointOnFace;
	}
	std::vector<glm::vec3> Clip::VertsOnEdge(const glm::vec3 edgeNorm, const glm::vec3 edgeVert, const std::vector<glm::vec3>& pointsToClip)
	{
		std::vector<glm::vec3> points;
	
		uint32_t startIdx = pointsToClip.size() - 1;
		uint32_t endIdx = 0;
		float sdStart = SignedDistance(edgeNorm, edgeVert, pointsToClip[startIdx]);
	
		while (endIdx < pointsToClip.size())
		{
			float sdEnd = SignedDistance(edgeNorm, edgeVert, pointsToClip[endIdx]);
	
			if (sdStart > 0)
			{
				if (sdEnd <= 0)
				{
					points.push_back(Interpolate(sdStart, sdEnd, pointsToClip[startIdx], pointsToClip[endIdx]));
					points.push_back(pointsToClip[endIdx]);
				}
			}
			else
			{
				if (sdEnd > 0)
					points.push_back(Interpolate(sdEnd, sdStart, pointsToClip[endIdx], pointsToClip[startIdx]));
				else
					points.push_back(pointsToClip[endIdx]);
			}
	
			startIdx = endIdx++;
			sdStart = sdEnd;
		}
	
		return points;
	}
		std::vector<glm::vec3> Clip::FaceOnPlane(const glm::vec3& planeNorm, const glm::vec3& planeVert, const std::vector<uint32_t>& faceVerts, const std::vector<glm::vec3>& verts)
	{
		std::vector<glm::vec3> points;
	
		uint32_t startIdx = faceVerts.size() - 1;
		uint32_t endIdx = 0;
		auto getVerts = [&](uint32_t idx)
		{
			return verts[idx]; // +vertOffs;
		};
		float sdStart = SignedDistance(planeNorm, planeVert, getVerts(faceVerts[startIdx]));
		while (endIdx < faceVerts.size())
		{
			float sdEnd = SignedDistance(planeNorm, planeVert, getVerts(faceVerts[endIdx]));
	
			if (sdStart > 0)
			{
				if (sdEnd <= 0)
				{
					points.push_back(Interpolate(sdStart, sdEnd, getVerts(faceVerts[startIdx]), getVerts(faceVerts[endIdx])));
					//points.push_back(getVerts(faceVerts[endIdx]) - planeNorm * sdEnd);
				}
			}
			else
			{
				if (sdEnd >= 0)
				{
					points.push_back(Interpolate(sdEnd, sdStart, getVerts(faceVerts[endIdx]), getVerts(faceVerts[startIdx])));
				}
				else
				{
					// points.push_back(getVerts(faceVerts[endIdx]) - planeNorm * sdEnd);
				}
			}
	
			startIdx = endIdx++;
			sdStart = sdEnd;
		}
	
		return points;
	}
	std::vector<glm::vec3> Clip::PointsOnPlane(const glm::vec3& planeNorm, const glm::vec3& planeVert, const std::vector<glm::vec3>& points)
{
	std::vector<glm::vec3> clipped;

	uint32_t startIdx = points.size() - 1;
	uint32_t endIdx = 0;
	float sdStart = SignedDistance(planeNorm, planeVert, points[startIdx]);
	while (endIdx < points.size())
	{
		float sdEnd = SignedDistance(planeNorm, planeVert, points[endIdx]);

		if (sdStart > 0)
		{
			if (sdEnd <= 0)
			{
				clipped.push_back(Interpolate(sdStart, sdEnd, points[startIdx], points[endIdx]));
				//points.push_back(getVerts(faceVerts[endIdx]) - planeNorm * sdEnd);
			}
		}
		else
		{
			if (sdEnd >= 0)
			{
				clipped.push_back(Interpolate(sdEnd, sdStart, points[endIdx], points[startIdx]));
			}
			else
			{
				// points.push_back(getVerts(faceVerts[endIdx]) - planeNorm * sdEnd);
			}
		}

		startIdx = endIdx++;
		sdStart = sdEnd;
	}

	return clipped;
}
	std::vector<glm::vec3> Clip::FaceOnFace(const Collider& facecollider, const Collider& facecollider2, const uint32_t faceIdx, const uint32_t faceIdx2, bool clipDepth)
	{
		std::vector<glm::vec3> result;
		const glm::vec3& faceNorm = facecollider.faces[faceIdx].norm;
		auto faceEdgeIdxs = facecollider.EdgesOnFace(faceIdx);
	
		std::vector<glm::vec3> contactPointsForFace;
		if (clipDepth)
		{
			contactPointsForFace = Clip::FaceOnPlane(faceNorm,
				facecollider.verts[facecollider.faces[faceIdx].vertIdx],
				facecollider2.faceVerts[faceIdx2], facecollider2.verts);
		}
		else
		{
			for (auto& vertIdx : facecollider2.faceVerts[faceIdx2])
			{
				contactPointsForFace.push_back(facecollider2.verts[vertIdx]);
			}
		}
	
		for (size_t j = 0; j < faceEdgeIdxs.size(); j++)
		{
			if (contactPointsForFace.size() == 0)
				break;
	
			const Edge& edge = facecollider.edges[faceEdgeIdxs[j]];
			glm::vec3 edgeNormal = glm::cross(faceNorm, facecollider.verts[edge.first] - facecollider.verts[edge.second]);
	
			{ // making sure the edgeNormal is pointing outwards
				glm::vec3 thirdVertOnFace;
				for (size_t k = 0; k < 3; k++)
				{
					uint32_t vertIdx = facecollider.faceVerts[faceIdx][k];
					if (vertIdx != edge.first &&
						vertIdx != edge.second)
					{
						thirdVertOnFace = facecollider.verts[vertIdx];
						break;
					}
				}
				if (glm::dot(edgeNormal, (facecollider.verts[edge.first]) - thirdVertOnFace) < 0)
					edgeNormal *= -1;
			}
			contactPointsForFace = Clip::VertsOnEdge(edgeNormal, facecollider.verts[edge.first], contactPointsForFace);
		}
		for (size_t j = 0; j < contactPointsForFace.size(); j++)
		{
			for (size_t k = 0; k < result.size(); k++)
			{
				auto vec = contactPointsForFace[j] - result[k];
				if (glm::dot(vec, vec) < 0.001f)
					goto AlreadyIn;
			}
			result.push_back(contactPointsForFace[j]);
		AlreadyIn:;
		}
		return result;
	}
	std::vector<glm::vec3> Clip::PointsOnFace(const Collider& facecollider, const uint32_t faceIdx, const std::vector<glm::vec3>& points)
	{
		auto contactPointsForFace = points;
		const glm::vec3& faceNorm = facecollider.faces[faceIdx].norm;
		auto faceEdgeIdxs = facecollider.EdgesOnFace(faceIdx);

		for (size_t j = 0; j < faceEdgeIdxs.size(); j++)
		{
			if (contactPointsForFace.size() == 0)
				break;

			const Edge& edge = facecollider.edges[faceEdgeIdxs[j]];
			glm::vec3 edgeNormal = glm::cross(faceNorm, facecollider.verts[edge.first] - facecollider.verts[edge.second]);

			{ // making sure the edgeNormal is pointing outwards
				glm::vec3 thirdVertOnFace;
				for (size_t k = 0; k < 3; k++)
				{
					uint32_t vertIdx = facecollider.faceVerts[faceIdx][k];
					if (vertIdx != edge.first &&
						vertIdx != edge.second)
					{
						thirdVertOnFace = facecollider.verts[vertIdx];
						break;
					}
				}
				if (glm::dot(edgeNormal, (facecollider.verts[edge.first]) - thirdVertOnFace) < 0)
					edgeNormal *= -1;
			}

			contactPointsForFace = Clip::VertsOnEdge(edgeNormal, facecollider.verts[edge.first], contactPointsForFace);
		}
		return contactPointsForFace;
	}
}