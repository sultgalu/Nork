module Nork.Physics;

#include "../Config.h"

namespace Nork::Physics
{
	Pipeline::Pipeline()
	{
		collisionDetector = new CollisionDetectionCPU(world);
	}
	void Pipeline::Update(float delta)
	{
		Timer summaT;
		Timer t;
		if (detectCollisions)
		{
			collisionDetector->BroadPhase();
			collisionDetector->NarrowPhase();
		}
		if (genContactPoints)
		{
			GenContactPoints();
		}
		if (handleCollisions)
		{
			ResolveCollisions(delta);
		}

		if (updateVelocities)
		{
			VelocityUpdate(delta); // can't put front couse it won't change Shape's verts pos.
		}
		if (updateRotation)
		{
			RotationUpdate(delta);
		}
	}

	glm::vec3 GetIntersectionPoint(glm::vec3 p1, glm::vec3 p2, glm::vec3 v1, glm::vec3 v2)
	{
		uint32_t axis1 = 0;
		uint32_t axis2 = 1;

		float divisor = v1[axis1] * v2[axis2] - v1[axis2] * v2[axis1];
		if (divisor == 0)
		{
			axis1 = 2;
			float divisor = v1[axis1] * v2[axis2] - v1[axis2] * v2[axis1];
			if (divisor == 0)
			{
				axis2 = 0;
				float divisor = v1[axis1] * v2[axis2] - v1[axis2] * v2[axis1];
			}
		}

		float a = (v2[axis1] * (p1[axis2] - p2[axis2]) - v2[axis2] * (p1[axis1] - p2[axis1])) / (v1[axis1] * v2[axis2] - v1[axis2] * v2[axis1]);
		// float b = (p1.x - p2.x + a * v1.x) / v2.x;
		auto res = glm::vec3(p1 + v1 * a);
		if (glm::isnan(res).x)
		{
			Logger::Debug("");
		}
		return res;
	}
	static glm::vec3 Interpolate(float sdStart, float sdEnd, const glm::vec3& start, const glm::vec3& end)
	{
		float normalDistance = sdStart - sdEnd;
		float startToFacePortion = sdStart / normalDistance;
		glm::vec3 startToEnd = end - start;
		glm::vec3 startToFace = startToEnd * startToFacePortion;
		glm::vec3 pointOnFace = start + startToFace;
		return pointOnFace;
	}
	std::vector<glm::vec3> ClipVertsOnEdge(glm::vec3 edgeNorm, glm::vec3 edgeVert, std::span<glm::vec3> pointsToClip)
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
	std::vector<glm::vec3> ClipFaceOnPlane(glm::vec3& planeNorm, glm::vec3& planeVert, std::span<uint32_t> faceVerts, std::span<glm::vec3> verts)
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
	std::vector<glm::vec3> ClipPointsOnPlane(glm::vec3& planeNorm, glm::vec3& planeVert, const std::vector<glm::vec3>& points)
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
	std::vector<glm::vec3> ClipFaceOnFace(const Shape& faceShape, const Shape& faceShape2, const uint32_t faceIdx, const uint32_t faceIdx2, bool clipDepth = true)
	{
		std::vector<glm::vec3> result;
		glm::vec3& faceNorm = faceShape.faces[faceIdx].norm;
		auto faceEdgeIdxs = faceShape.EdgesOnFace(faceIdx);

		std::vector<glm::vec3> contactPointsForFace;
		if (clipDepth)
		{
			contactPointsForFace = ClipFaceOnPlane(faceNorm,
				faceShape.verts[faceShape.faces[faceIdx].vertIdx],
				faceShape2.faceVerts[faceIdx2], faceShape2.verts);
		}
		else
		{
			for (auto& vertIdx : faceShape2.faceVerts[faceIdx2])
			{
				contactPointsForFace.push_back(faceShape2.verts[vertIdx]);
			}
		}

		for (size_t j = 0; j < faceEdgeIdxs.size(); j++)
		{
			if (contactPointsForFace.size() == 0)
				break;

			Edge& edge = faceShape.edges[faceEdgeIdxs[j]];
			glm::vec3 edgeNormal = glm::cross(faceNorm, faceShape.verts[edge.first] - faceShape.verts[edge.second]);

			{ // making sure the edgeNormal is pointing outwards
				glm::vec3 thirdVertOnFace;
				for (size_t k = 0; k < 3; k++)
				{
					uint32_t vertIdx = faceShape.faceVerts[faceIdx][k];
					if (vertIdx != edge.first &&
						vertIdx != edge.second)
					{
						thirdVertOnFace = faceShape.verts[vertIdx];
						break;
					}
				}
				if (glm::dot(edgeNormal, (faceShape.verts[edge.first]) - thirdVertOnFace) < 0)
					edgeNormal *= -1;
			}
			contactPointsForFace = ClipVertsOnEdge(edgeNormal, faceShape.verts[edge.first], contactPointsForFace);
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
	std::vector<glm::vec3> ClipPointsOnFace(const Shape& faceShape, const uint32_t faceIdx, const std::vector<glm::vec3>& points)
	{
		auto contactPointsForFace = points;
		glm::vec3& faceNorm = faceShape.faces[faceIdx].norm;
		auto faceEdgeIdxs = faceShape.EdgesOnFace(faceIdx);

		for (size_t j = 0; j < faceEdgeIdxs.size(); j++)
		{
			if (contactPointsForFace.size() == 0)
				break;

			Edge& edge = faceShape.edges[faceEdgeIdxs[j]];
			glm::vec3 edgeNormal = glm::cross(faceNorm, faceShape.verts[edge.first] - faceShape.verts[edge.second]);

			{ // making sure the edgeNormal is pointing outwards
				glm::vec3 thirdVertOnFace;
				for (size_t k = 0; k < 3; k++)
				{
					uint32_t vertIdx = faceShape.faceVerts[faceIdx][k];
					if (vertIdx != edge.first &&
						vertIdx != edge.second)
					{
						thirdVertOnFace = faceShape.verts[vertIdx];
						break;
					}
				}
				if (glm::dot(edgeNormal, (faceShape.verts[edge.first]) - thirdVertOnFace) < 0)
					edgeNormal *= -1;
			}

			contactPointsForFace = ClipVertsOnEdge(edgeNormal, faceShape.verts[edge.first], contactPointsForFace);
		}
		return contactPointsForFace;
	}

	std::vector<glm::vec3> FaceContactPoints(const Shape& faceShape, const Shape& vertShape, const uint32_t faceIdx, const uint32_t vertIdx)
	{
		auto& faceNormal = faceShape.faces[faceIdx].norm;
		auto& faceVert = faceShape.verts[faceShape.faces[faceIdx].vertIdx];

		auto sideFaces = vertShape.SideFacesOfVert(vertIdx);
		auto sideEdges = vertShape.Edges(vertShape.verts[vertIdx]);

		//auto faceEdgeIdxs = faceShape.EdgesOnFace(faceIdx);
		constexpr float faceBias = 0.0f;
		constexpr float edgeBias = 90.0f;
		for (auto& sideFaceIdx : sideFaces)
		{
			auto& sideFaceNorm = vertShape.faces[sideFaceIdx].norm;
			auto cos = glm::dot(faceNormal, glm::normalize(-sideFaceNorm));
			auto degrees = glm::degrees(glm::acos(cos));
			if (glm::abs(degrees) <= faceBias)
			{
				std::vector<glm::vec3> facePoints;
				for (auto& vertIdx : vertShape.faceVerts[sideFaceIdx])
				{
					facePoints.push_back(vertShape.verts[vertIdx]);
				}
				return ClipPointsOnFace(faceShape, faceIdx, facePoints);
			}
		}
		for (auto& sideEdgeIdx : sideEdges)
		{
			auto edgeVec = vertShape.verts[sideEdgeIdx.first] - vertShape.verts[sideEdgeIdx.second];
			auto dotProd = glm::abs(glm::dot(faceNormal, edgeVec));
			auto degrees = glm::degrees(glm::acos(dotProd));
			if (degrees >= edgeBias)
			{	
				auto clipped = ClipPointsOnFace(faceShape, faceIdx, { vertShape.verts[sideEdgeIdx.first], vertShape.verts[sideEdgeIdx.second] });
				if (!clipped.empty())
				{
					return clipped;
				}
			}
		}
		if (!ClipPointsOnFace(faceShape, faceIdx, { vertShape.verts[vertIdx] }).empty())
		{
			return { vertShape.verts[vertIdx] };
		}
		for (auto& sideEdgeIdx : sideEdges)
		{
			auto clipped = ClipPointsOnPlane(faceNormal, faceVert, { vertShape.verts[sideEdgeIdx.first], vertShape.verts[sideEdgeIdx.second] }); // max 1
			if (!clipped.empty())
			{
				return clipped;
			}
		}
		Logger::Error("");
		return {};
	}
	void Pipeline::GenContactPoints()
	{
		contactPoints.clear();
		auto& colls = collisionDetector->GetResults();
		for (size_t i = 0; i < colls.size(); i++)
		{
			auto& res = colls[i];
			float bias = 0.1f;
			if (res.depth < 0)
			{
				contactPoints.push_back(glm::vec3(0));
				continue;
			}
			auto shape1 = world.shapes[res.pair.x];
			auto shape2 = world.shapes[res.pair.y];


			if (res.type == CollisionType::FaceVert)
			{
				auto cps = FaceContactPoints(shape1, shape2, res.featureIdx1, res.featureIdx2);
				glm::vec3 contactPoint = Center(cps);
				if (glm::isnan(contactPoint.x))
					Logger::Error("");
				contactPoints.push_back(contactPoint);
			}
			else if (res.type == CollisionType::VertFace)
			{
				auto contact = FaceContactPoints(shape2, shape1, res.featureIdx1, res.featureIdx2);
				glm::vec3 contactPoint = Center(contact);
				if (glm::isnan(contactPoint.x))
					Logger::Error("");
				contactPoints.push_back(contactPoint);
			}
			else if (res.type == CollisionType::EdgeEdge)
			{
				auto& edge1 = shape1.edges[res.featureIdx1];
				auto& edge2 = shape2.edges[res.featureIdx2];

				glm::vec3 edge1Dir = shape1.EdgeDirection(edge1);
				glm::vec3 edge2Dir = shape2.EdgeDirection(edge2);

				glm::vec3 edgePoint1 = shape1.FirstVertFromEdge(edge1);
				glm::vec3 edgePoint2 = shape2.FirstVertFromEdge(edge2);

				glm::vec3 planeNorm = res.dir;
				float sd = SignedDistance(planeNorm, edgePoint1, edgePoint2);
				glm::vec3 edgePoint2Translated = edgePoint2 - sd * planeNorm;

				glm::vec3 contactPointOnEdge1 = GetIntersectionPoint(edgePoint1, edgePoint2Translated, edge1Dir, edge2Dir);
				glm::vec3 contactPoint = contactPointOnEdge1; //+sd * 0.5f * planeNorm;
				contactPoints.push_back(contactPointOnEdge1);
			}
		}
	}
	struct Island
	{
		std::unordered_set<int> objects;
		bool isStatic = false; // has a static object
		KinematicData kinem;
	};
	void Pipeline::ResolveCollisions(float delta)
	{
		auto& colls = collisionDetector->GetResults();
		for (size_t i = 0; i < colls.size(); i++)
		{
			auto& coll = colls[i];
			if (coll.depth < 0.0f)
				continue;
			auto& objs = colls[i].pair;
			auto& kinem1 = world.kinems[objs.x];
			auto& kinem2 = world.kinems[objs.y];

			// if (kinem1.isStatic)
			// 	kinem1.mass = 100000000.0f;
			// if (kinem2.isStatic)
			// 	kinem2.mass = 100000000.0f;

			glm::vec3 t = glm::vec3(0);
			float fr = -0.9f;
			float fr2 = 1.0f;
			float bias = 0.1f;
			const glm::vec3& contactPoint = contactPoints[i];
			glm::vec3 r1 = contactPoint - world.shapes[objs.x].center;
			glm::vec3 r2 = contactPoint - world.shapes[objs.y].center;
			auto vel1 = kinem1.velocity + glm::cross(kinem1.w, r1);
			auto vel2 = kinem2.velocity + glm::cross(kinem2.w, r2);
			auto velDiffLenSquared = glm::dot(vel1 - vel2, vel1 - vel2);

			auto applyForce = [&](KinematicData& kinem, const glm::vec3& r, const glm::vec3& collDir)
			{
				if (glm::dot(kinem.velocity, kinem.forces) >= 0)
				{
					// auto forceDirVel = glm::dot(kinem.forces, kinem.velocity);
					// if (forceDirVel < 0.001f)
					// {
					// 	kinem.velocity -= forceDirVel * kinem.forces;
					// }
					auto counterForce = glm::dot(kinem.forces, collDir);
					auto perpForce = (kinem.forces - counterForce) * fr2;
					auto counterTorque = glm::cross(r, counterForce * kinem.forces);
					auto frictionTorque = glm::cross(r, perpForce * kinem.forces);
					// kinem.forces += counterForce * collDir;
					// kinem.forces += perpForce * (kinem.forces - collDir * counterForce);
					kinem.torque += counterTorque;
					// kinem.torque += frictionTorque;
					if (glm::isnan(kinem.torque.x))
						Logger::Error("");

				}
			};
			// applyForce(kinem1, r1, coll.dir);
			// applyForce(kinem2, r2, coll.dir);

			// if (velDiffLenSquared < 0.01f)
			// {
			// 	kinem1.velocity -= coll.dir * glm::dot(kinem1.velocity, coll.dir);
			// 	kinem2.velocity -= coll.dir * glm::dot(kinem2.velocity, coll.dir);
			// 	kinem1.w = coll.dir * glm::dot(kinem1.w, coll.dir);
			// 	kinem2.w = coll.dir * glm::dot(kinem2.w, coll.dir);
			// }
			if (coll.depth == 0)
			{
				coll.depth = 0;
			}
			if (coll.depth > 0)
			{
				float actualCoefficient = coefficient;
				if (velDiffLenSquared < 0.01f)
				{
					actualCoefficient = 0;
					// fr = 0.0f;
				}

				float resulting = (1 + actualCoefficient) * glm::dot(kinem1.velocity - kinem2.velocity, coll.dir) / (1 / kinem1.mass + 1 / kinem2.mass /* + angularPart*/);

				auto angularPart1 = glm::cross((1 / kinem1.I) * (glm::cross(r1, coll.dir)), r1);
				auto angularPart2 = glm::cross((1 / kinem2.I) * (glm::cross(r2, coll.dir)), r2);

				float angularPart = glm::dot(angularPart1 + angularPart2, coll.dir);

				t = vel1 - vel2;
				if (t != glm::vec3(0))
				{
					t = glm::cross(coll.dir, glm::normalize(t));
					t = glm::cross(t, coll.dir);
				}

				float resultingA = -(1 + actualCoefficient) * glm::dot(vel1 - vel2, coll.dir) / (1 / kinem1.mass + 1 / kinem2.mass + angularPart);
				resulting = resultingA;
				glm::vec3 deltaW1 = (1.0f / kinem1.I + (kinem2.isStatic ? 1.0f / kinem2.I : 0)) * glm::cross(r1, resultingA * (coll.dir + fr * t));
				glm::vec3 deltaW2 = -(1.0f / kinem2.I + (kinem1.isStatic ? 1.0f / kinem1.I : 0)) * glm::cross(r2, resultingA * (coll.dir + fr * t));
				if (glm::isnan(deltaW1.x) || glm::isnan(deltaW2.x))
					Logger::Error("");
				if (kinem1.isStatic)
				{
					deltaW1 = glm::vec3(0);
				}
				if (kinem2.isStatic)
				{
					deltaW2 = glm::vec3(0);
				}
				kinem1.w += deltaW1;
				kinem2.w += deltaW2;

				float deltaV1 = 1.0f / kinem1.mass + (kinem2.isStatic ? 1.0f / kinem2.mass : 0);
				float deltaV2 = -(1.0f / kinem2.mass + (kinem1.isStatic ? 1.0f / kinem1.mass : 0));
				if (kinem1.isStatic)
				{
					deltaV1 = 0;
				}
				if (kinem2.isStatic)
				{
					deltaV2 = 0;
				}
				if (coll.depth > 0 && glm::dot(coll.dir, kinem1.velocity - kinem2.velocity) <= 0)
				{
					kinem1.velocity += resulting * deltaV1 * (coll.dir + fr * t);
					kinem2.velocity += resulting * deltaV2 * (coll.dir + fr * t);
				}

				if (glm::isnan(kinem1.velocity.x) || glm::isnan(kinem2.velocity.x))
					Logger::Error("");

				auto newVel1 = kinem1.velocity + glm::cross(kinem1.w, r1);
				auto newVel2 = kinem2.velocity + glm::cross(kinem2.w, r2);

				auto cr = -glm::dot((newVel1 - newVel2), coll.dir) / glm::dot((vel1 - vel2), coll.dir);
				if (cr == actualCoefficient)
				{
					actualCoefficient = 0;
				}
				else
				{
					actualCoefficient = 0;
				}
			}
			if (coll.depth > 0)
			{
				if (!kinem1.isStatic && kinem2.isStatic)
				{
					glm::vec3 translate = coll.dir * coll.depth;
					kinem1.position += translate;
				}
				else if (kinem1.isStatic && !kinem2.isStatic)
				{
					glm::vec3 translate = -coll.dir * coll.depth;
					kinem2.position += translate;
				}
				else if (!kinem1.isStatic && !kinem2.isStatic)
				{
					glm::vec3 translate = coll.dir * (coll.depth / 2);
					kinem1.position += translate;
					kinem2.position -= translate;
				}
			}
		}
	}
	void Pipeline::RotationUpdate(float delta)
	{
		for (auto& k: world.kinems)
		{
			if (k.isStatic || glm::dot(k.w, k.w) == 0) continue;

			float angle = glm::length(k.w);
			if (angle > 0)
			{
				glm::quat rot(2, k.w * delta);
				rot = glm::normalize(rot);
				k.quaternion = rot * k.quaternion;

				if (glm::isnan(k.torque.x))
					Logger::Error("");
				glm::vec3 angularAcc = k.torque / k.I;
				k.w += angularAcc * delta;
				k.torque = glm::vec3(0);
			}
		}
	}

	void Pipeline::SetColliders(std::span<Collider> colls)
	{
		world.ClearColliderData();
		for (size_t i = 0; i < colls.size(); i++)
		{
			world.AddCollider(colls[i]);
		}
		this->colls = std::vector<Collider>(colls.begin(), colls.end());
	}

	void Pipeline::SetModels(std::span<glm::vec3> translate, std::span<glm::quat> quaternions)
	{
		Timer t;
		collisionDetector->UpdateTransforms(translate, quaternions);
	}

	void Pipeline::VelocityUpdate(float delta)
	{
		for (size_t i = 0; i < world.kinems.size(); i++)
		{
			if (world.kinems[i].isStatic) continue;
			auto& k = world.kinems[i];

			glm::vec3 acceleration = k.forces / k.mass;
			auto deltaV = acceleration * delta;
			glm::vec3 translate = k.velocity * delta + 0.5f * acceleration * delta * delta;

			k.velocity += deltaV;
			if (glm::isnan(k.velocity.x))
				Logger::Error("");
			k.position += translate;
			k.forces = g * k.mass * glm::vec3(0, -1, 0);
		}
	}
}