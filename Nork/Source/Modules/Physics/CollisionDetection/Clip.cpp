#include "pch.h"
#include "Clip.h"
#include "../Utils.h"

namespace Nork::Physics
{
	static glm::vec3 PointOnFace(float sdStart, float sdEnd, glm::vec3& start, glm::vec3& end)
	{
		float normalDistance = sdStart - sdEnd;
		float startToFacePortion = sdStart / normalDistance;
		glm::vec3 startToEnd = end - start;
		glm::vec3 startToFace = startToEnd * startToFacePortion;
		glm::vec3 pointOnFace = start + startToFace;
		return pointOnFace;
	}

	std::pair<std::vector<uint32_t>, std::vector<glm::vec3>> Clip::SutherlandHodgmanClip(Face& face1, glm::vec3& faceNormal1, uint32_t point2)                                                                             
	{
		glm::vec3 c1RefPoint = shape1.SomePointOnFace(face1);
		std::vector<uint32_t> savedPoints; // inner points mostly
		std::vector<glm::vec3> generatedPoints; // generated onto face1

		for (auto& face2 : shape2.faces)
		{
			for (size_t i = 0; i < face2.size(); i++)
				if (face2[i] == point2) goto Contains;
			continue; // face does not contain point, no clipping
		Contains:;

			for (size_t i = 1; i < face2.size(); i++)
			{
				uint32_t startIdx = face2[i - 1];
				uint32_t endIdx = face2[i];
				glm::vec3& start = shape2.verts[startIdx];
				glm::vec3& end = shape2.verts[endIdx];

				float sdStart = SignedDistance(faceNormal1, c1RefPoint, start);
				float sdEnd = SignedDistance(faceNormal1, c1RefPoint, end);

				if (sdStart > 0)
				{
					if (sdEnd > 0)
					{
						continue;
					}
					else
					{
						savedPoints.push_back(endIdx);
						generatedPoints.push_back(PointOnFace(sdStart, sdEnd, start, end));
					}
				}
				else
				{
					if (sdEnd > 0)
					{
						generatedPoints.push_back(PointOnFace(sdEnd, sdStart, end, start));
					}
					else
					{
						savedPoints.push_back(endIdx);
					}
				}
			}

			{
				uint32_t startIdx = face2[face2.size() - 1];
				uint32_t endIdx = face2[0];
				glm::vec3& start = shape2.verts[startIdx];
				glm::vec3& end = shape2.verts[endIdx];

				float sdStart = SignedDistance(faceNormal1, c1RefPoint, start);
				float sdEnd = SignedDistance(faceNormal1, c1RefPoint, end);

				if (sdStart > 0)
				{
					if (sdEnd > 0)
					{
						continue;
					}
					else
					{
						savedPoints.push_back(endIdx);
						generatedPoints.push_back(PointOnFace(sdStart, sdEnd, start, end));
					}
				}
				else
				{
					if (sdEnd > 0)
					{
						generatedPoints.push_back(PointOnFace(sdEnd, sdStart, end, start));
					}
					else
					{
						savedPoints.push_back(endIdx);
					}
				}
			}
		}

		return std::pair(savedPoints, generatedPoints);
	}
	// points2 should be ordered. (Don't care till using triangles as faces)
	std::pair<std::vector<uint32_t>, std::vector<glm::vec3>> SutherlandHodgmanClipFaces(glm::vec3& pointOnFace1, glm::vec3& face1Normal, IndexedVerts points2)
	{
		glm::vec3& c1RefPoint = pointOnFace1;
		std::vector<uint32_t> savedPoints; // inner points mostly
		std::vector<glm::vec3> generatedPoints; // generated onto face1

		for (size_t i = 1; i < points2.size(); i++)
		{
			uint32_t startIdx = i - 1;
			uint32_t endIdx = i;
			glm::vec3& start = points2[startIdx];
			glm::vec3& end = points2[endIdx];

			// OPTIMIZE:: sdStart has been calulated as sdEnd prevously
			float sdStart = SignedDistance(face1Normal, c1RefPoint, start);
			float sdEnd = SignedDistance(face1Normal, c1RefPoint, end);

			if (sdStart > 0)
			{
				if (sdEnd > 0)
				{
					continue;
				}
				else
				{
					savedPoints.push_back(points2.GetPointIdx(endIdx));
					generatedPoints.push_back(PointOnFace(sdStart, sdEnd, start, end));
				}
			}
			else
			{
				if (sdEnd > 0)
				{
					generatedPoints.push_back(PointOnFace(sdEnd, sdStart, end, start));
				}
				else
				{
					savedPoints.push_back(points2.GetPointIdx(endIdx));
				}
			}

			//if (sdEnd <= 0)
			//	savedPoints.push_back(points2.GetPointIdx(endIdx));
			//if (sdEnd * sdStart < 0)
			//	generatedPoints.push_back(PointOnFace(sdStart, sdEnd, start, end)); // check order
		}

		{
			uint32_t startIdx = points2.size() - 1;
			uint32_t endIdx = 0;
			glm::vec3& start = points2[startIdx];
			glm::vec3& end = points2[endIdx];

			float sdStart = SignedDistance(face1Normal, c1RefPoint, start);
			float sdEnd = SignedDistance(face1Normal, c1RefPoint, end);

			if (sdStart > 0)
			{
				if (sdEnd > 0)
				{
					;
				}
				else
				{
					savedPoints.push_back(endIdx);
					generatedPoints.push_back(PointOnFace(sdStart, sdEnd, start, end));
				}
			}
			else
			{
				if (sdEnd > 0)
				{
					generatedPoints.push_back(PointOnFace(sdEnd, sdStart, end, start));
				}
				else
				{
					savedPoints.push_back(endIdx);
				}
			}
		}

		return std::pair(savedPoints, generatedPoints);
	}
	std::vector<glm::vec3> SutherlandHodgmanClipEdge(glm::vec3& edgeNormal, std::pair<glm::vec3&, glm::vec3&> edgePoints1, std::span<glm::vec3> points2)
	{
		std::vector<uint32_t> savedPoints; // inner points mostly
		std::vector<glm::vec3> generatedPoints; // generated onto face1
		std::vector<glm::vec3> saved; // generated onto face1

		for (size_t i = 1; i < points2.size(); i++)
		{
			uint32_t startIdx = i - 1;
			uint32_t endIdx = i;
			glm::vec3& start = points2[startIdx];
			glm::vec3& end = points2[endIdx];

			// OPTIMIZE:: sdStart has been calulated as sdEnd prevously
			float sdStart = SignedDistance(edgeNormal, edgePoints1.first, start);
			float sdEnd = SignedDistance(edgeNormal, edgePoints1.first, end);

			if (sdStart > 0)
			{
				if (sdEnd > 0)
				{
					continue;
				}
				else
				{
					saved.push_back(points2[endIdx]);
					saved.push_back(PointOnFace(sdStart, sdEnd, start, end));

					savedPoints.push_back(endIdx);
					generatedPoints.push_back(PointOnFace(sdStart, sdEnd, start, end));
				}
			}
			else
			{
				if (sdEnd > 0)
				{
					saved.push_back(PointOnFace(sdStart, sdEnd, start, end));

					generatedPoints.push_back(PointOnFace(sdEnd, sdStart, end, start));
				}
				else
				{
					saved.push_back(points2[endIdx]);

					savedPoints.push_back(endIdx);
				}
			}

			//if (sdEnd <= 0)
			//	savedPoints.push_back(endIdx);
			//if (sdEnd * sdStart < 0)
			//	generatedPoints.push_back(PointOnFace(sdStart, sdEnd, start, end)); // check order
		}

		{
			uint32_t startIdx = points2.size() - 1;
			uint32_t endIdx = 0;
			glm::vec3& start = points2[startIdx];
			glm::vec3& end = points2[endIdx];

			float sdStart = SignedDistance(edgeNormal, edgePoints1.first, start);
			float sdEnd = SignedDistance(edgeNormal, edgePoints1.first, end);

			if (sdStart > 0)
			{
				if (sdEnd > 0)
				{
					;
				}
				else
				{
					saved.push_back(points2[endIdx]);
					saved.push_back(PointOnFace(sdStart, sdEnd, start, end));

					savedPoints.push_back(endIdx);
					generatedPoints.push_back(PointOnFace(sdStart, sdEnd, start, end));
				}
			}
			else
			{
				if (sdEnd > 0)
				{
					saved.push_back(PointOnFace(sdStart, sdEnd, start, end));

					generatedPoints.push_back(PointOnFace(sdEnd, sdStart, end, start));
				}
				else
				{
					saved.push_back(points2[endIdx]);

					savedPoints.push_back(endIdx);
				}
			}
		}

		return saved;
	}
	std::vector<glm::vec3> SutherlandHodgmanClipEdges(glm::vec3& pointInsideFace, glm::vec3& faceNormal, Edges edges, std::vector<glm::vec3> points2)
	{
		std::vector<glm::vec3> saved = points2;

		for (size_t i = 0; i < edges.EdgeCount(); i++)
		{
			if (saved.size() == 0)
				return {};

			glm::vec3 edgeNormal = glm::cross(faceNormal, edges[i].first - edges[i].second);
			if (glm::dot(edgeNormal, edges[i].first - pointInsideFace) < 0)
				edgeNormal *= -1;

			saved = SutherlandHodgmanClipEdge(edgeNormal, edges[i], saved);
		}

		return saved;
	}
	std::vector<glm::vec3> Clip::GetAllContactPointsBF()
	{
		std::vector<glm::vec3> result;

		// OPTIMIZE:: could filter faces that are the other side of the centerplane (plane facing the other obj's center)
		for (auto& face1 : shape1.faces)
		{
			for (auto& face2 : shape2.faces)
			{
				auto faceClipRes = SutherlandHodgmanClipFaces(shape1.verts[face1[0]], shape1.FaceNormal(face1), IndexedVerts(face2, shape2.verts));

				std::vector<glm::vec3>& resultPoints = faceClipRes.second;
				// don't care about "saved points" (they are inside the plane, not in contact with it)
				/*for (size_t i = 0; i < faceClipRes.first.size(); i++)
				{
					resultPoints.push_back(shape2.verts[faceClipRes.first[i]]);
				}*/
				if (resultPoints.size() == 0)
					continue;

				glm::vec3 faceCenter = shape1.FaceCenter(face1);
				auto edgesForFace1 = shape1.Edges(face1);

				auto edgeClipRes = SutherlandHodgmanClipEdges(faceCenter, shape1.FaceNormal(face1), Edges(edgesForFace1, shape1.verts), resultPoints);

				result.insert(result.end(), edgeClipRes.begin(), edgeClipRes.end());
			}
		}

		return result;
	}
	bool Clip::GetResult()
	{
		auto contactPoints = GetAllContactPointsBF();
		return contactPoints.size() > 0;
	}
}