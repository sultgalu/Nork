#include "PhysicsSystem.h"
#include "../Config.h"
#include "CollisionDetectionGPU.h"
#include "CollisionDetectionCPU.h"

namespace Nork::Physics
{
	System::System()
	{
		if constexpr (Config::CollisionDetectionGPU)
		{
			collisionDetector = new CollisionDetectionGPU();
		}
		else
		{
			collisionDetector = new CollisionDetectionCPU(world);
		}
	}
	void System::Update(float delta)
	{
		deltas.clear();
		Timer summaT;
		Timer t;
		if (detectCollisions)
		{
			if constexpr (Config::CollisionDetectionGPU)
				((CollisionDetectionGPU*)collisionDetector)->BroadPhase();
			else ((CollisionDetectionCPU*)collisionDetector)->BroadPhase();
			deltas.push_back(std::pair("broad phase", t.Reset()));

			if constexpr (Config::CollisionDetectionGPU)
				((CollisionDetectionGPU*)collisionDetector)->NarrowPhase();
			else ((CollisionDetectionCPU*)collisionDetector)->NarrowPhase();
			deltas.push_back(std::pair("narrow phase", t.Reset()));
		}
		if (genContactPoints)
		{
			GenContactPoints();
			deltas.push_back(std::pair("gen contact points", t.Reset()));
		}
		if (handleCollisions)
		{
			ResolveCollisions(delta);
			deltas.push_back(std::pair("resolve colls", t.Reset()));
		}

		if (updateVelocities)
		{
			VelocityUpdate(delta); // can't put front couse it won't change Shape's verts pos.
			deltas.push_back(std::pair("update vels", t.Reset()));
		}
		if (updateRotation)
		{
			RotationUpdate(delta);
			deltas.push_back(std::pair("update rots", t.Reset()));
		}
		deltas.push_back(std::pair("WHOLE", summaT.Reset()));
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
	static glm::vec3 Interpolate(float sdStart, float sdEnd, glm::vec3& start, glm::vec3& end)
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
		float sdStart = SignedDistance(planeNorm, planeVert, verts[faceVerts[startIdx]]);

		while (endIdx < faceVerts.size())
		{
			float sdEnd = SignedDistance(planeNorm, planeVert, verts[faceVerts[endIdx]]);

			if (sdStart > 0)
			{
				if (sdEnd <= 0)
				{
					points.push_back(Interpolate(sdStart, sdEnd, verts[faceVerts[startIdx]], verts[faceVerts[endIdx]]));
					points.push_back(verts[faceVerts[endIdx]] - planeNorm * sdEnd);
				}
			}
			else
			{
				if (sdEnd > 0)
					points.push_back(Interpolate(sdEnd, sdStart, verts[faceVerts[endIdx]], verts[faceVerts[startIdx]]));
				else
					points.push_back(verts[faceVerts[endIdx]] - planeNorm * sdEnd);
			}

			startIdx = endIdx++;
			sdStart = sdEnd;
		}

		return points;
	}
	std::vector<glm::vec3> FaceContactPoints(Shape& faceShape, Shape& vertShape, uint32_t faceIdx, uint32_t vertIdx)
	{
		std::vector<glm::vec3> result;

		glm::vec3& faceNorm = faceShape.faces[faceIdx].norm;
		auto sideFaces = vertShape.SideFacesOfVert(vertIdx);
		auto faceEdgeIdxs = faceShape.EdgesOnFace(faceIdx);
		for (size_t i = 0; i < sideFaces.size(); i++)
		{
			auto contactPointsForFace = ClipFaceOnPlane(faceNorm,
				faceShape.verts[faceShape.faces[faceIdx].vertIdx],
				vertShape.faceVerts[sideFaces[i]], vertShape.verts);

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
					if (glm::dot(edgeNormal, faceShape.verts[edge.first] - thirdVertOnFace) < 0)
						edgeNormal *= -1;
				}

				contactPointsForFace = ClipVertsOnEdge(edgeNormal, faceShape.verts[edge.first], contactPointsForFace);
			}
			for (size_t j = 0; j < contactPointsForFace.size(); j++)
			{
				for (size_t k = 0; k < result.size(); k++)
				{
					if (contactPointsForFace[j] == result[k])
						goto AlreadyIn;
				}
				result.push_back(contactPointsForFace[j]);
			AlreadyIn:;
			}
			//result.insert(result.end(), contactPointsForFace.begin(), contactPointsForFace.end());
		}
	
		return result;
	}

	void System::GenContactPoints()
	{
		contactPoints.clear();
		auto& colls = collisionDetector->GetResults();
		for (size_t i = 0; i < colls.size(); i++)
		{
			auto& res = colls[i];
			//if (res.distance <= 0) continue;
			auto shape1 = world.shapes[res.pair.x];
			auto shape2 = world.shapes[res.pair.y];

			if (res.type == CollisionType::FaceVert)
			{
				//auto& face1 = shape1.faces[res.featureIdx1];
				//auto& vert2 = shape2.verts[res.featureIdx2];
				//glm::vec3 contactPoint = vert2 + -res.dir * res.depth; // *0.5f;
				auto cps = FaceContactPoints(shape1, shape2, res.featureIdx1, res.featureIdx2);
				glm::vec3 contactPoint = Center(cps);
				contactPoints.push_back(contactPoint);
			}
			else if (res.type == CollisionType::VertFace)
			{
				auto& vert1 = shape1.verts[res.featureIdx2];
				auto& face2 = shape2.faces[res.featureIdx1];
				//glm::vec3 contactPoint = vert1 + res.dir * res.depth; //* 0.5f;

				glm::vec3 contactPoint = Center(FaceContactPoints(shape2, shape1, res.featureIdx1, res.featureIdx2));
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
	void System::ResolveCollisions2(float delta)
	{
		auto& colls = collisionDetector->GetResults();
		for (size_t i = 0; i < colls.size(); i++)
		{
			auto& coll = colls[i];
			if (coll.depth < -0.0001f)
				continue;
			auto& objs = colls[i].pair;
			auto& kinem1 = world.kinems[objs.x];
			auto& kinem2 = world.kinems[objs.y];

			if (coll.depth > 0)
			{
				if (glm::dot(coll.dir, kinem1.velocity - kinem2.velocity) > 0)
				{
					// they are already moving away from eachother. Was handled some frame earlier
				}
				else
				{
					glm::vec3 acceleration = kinem1.forces / kinem1.mass;
					auto ds1 = glm::dot(kinem1.velocity * delta + acceleration * delta * (delta / 2), coll.dir); // delta s
					auto errPct = ds1 == 0 ? 0 : coll.depth / ds1;
					auto dv1 = (kinem1.forces / kinem1.mass) * delta;
 					kinem1.velocity -= dv1 * glm::abs(errPct) + dv1;

					if (kinem1.isStatic)
					{
						glm::vec3 changeInVel = -(1 + coefficient) * glm::dot(kinem2.velocity, coll.dir) * -coll.dir;
						kinem2.velocity = kinem2.velocity + changeInVel;

						float vNextFrame = glm::dot(kinem2.forces * (delta * 1) / kinem2.mass, coll.dir);
						float vNow = glm::dot(changeInVel, coll.dir);
						if (glm::dot(vNextFrame, vNow) < 0 && glm::abs(vNextFrame) > glm::abs(vNow))
							kinem2.velocity -= glm::dot(kinem2.velocity, coll.dir) * coll.dir;
					}
					if (kinem2.isStatic)
					{
						glm::vec3 changeInVel = -(1 + coefficient) * glm::dot(kinem1.velocity, coll.dir) * coll.dir;
						kinem1.velocity = kinem1.velocity + changeInVel;

						float vNextFrame = glm::dot(kinem1.forces * (delta * 1) / kinem1.mass, coll.dir);
						float vNow = glm::dot(changeInVel, coll.dir);
						if (glm::dot(vNextFrame, vNow) < 0 && glm::abs(vNextFrame) > glm::abs(vNow))
							kinem1.velocity -= glm::dot(kinem1.velocity, coll.dir) * coll.dir;
					}
					else
					{
						float resulting = (1 + coefficient) * glm::dot(kinem1.velocity - kinem2.velocity, coll.dir) / (1 / kinem1.mass + 1 / kinem2.mass);
						float deltaV1 = -(resulting / kinem1.mass);
						float deltaV2 = (resulting / kinem2.mass);

						// this would be heplful for islands
						/*float deltaVDiff = deltaV2 - deltaV1;
						float deltaV1Next = glm::dot(kinem1.forces * (delta * 1) / kinem1.mass, coll.dir);
						float deltaV2Next = glm::dot(kinem2.forces * (delta * 1) / kinem2.mass, coll.dir);
						float deltaVDiffNext = deltaV2Next - deltaV1Next;

						if (deltaVDiff * deltaVDiffNext < 0 && glm::abs(deltaVDiff) < glm::abs(deltaVDiffNext))
						{
							float resulting = glm::dot(kinem1.velocity - kinem2.velocity, coll.dir) / (1 / kinem1.mass + 1 / kinem2.mass);
							deltaV1 = -(resulting / kinem1.mass);
							deltaV2 = (resulting / kinem2.mass);
						}*/

						kinem1.velocity = kinem1.velocity + deltaV1 * coll.dir;
						kinem2.velocity = kinem2.velocity + deltaV2 * coll.dir;
					}
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
			if (coll.depth >= 0)
			{
				glm::vec3 cf1 = coll.dir * kinem1.forces;
				glm::vec3 cf2 = -coll.dir * kinem2.forces;
				kinem1.forces -= cf1;
				kinem2.forces -= cf2;
			}
		}
	}
	void System::ResolveCollisions(float delta)
	{
		auto& colls = collisionDetector->GetResults();
		for (size_t i = 0; i < colls.size(); i++)
		{
			auto& coll = colls[i];
			if (coll.depth < -0.0001f)
				continue;
			auto& objs = colls[i].pair;
			auto& kinem1 = world.kinems[objs.x];
			auto& kinem2 = world.kinems[objs.y];

			if (coll.depth >= 0)
			{
				glm::vec3 acceleration = kinem1.forces / kinem1.mass;
				auto ds1 = glm::dot(kinem1.velocity * delta + acceleration * delta * (delta / 2), coll.dir); // delta s
				auto errPct = ds1 == 0 ? 0 : coll.depth / ds1;
				auto dv1 = (kinem1.forces / kinem1.mass) * delta;
				kinem1.velocity -= dv1 * glm::abs(errPct) + dv1;

				const glm::vec3& contactPoint = contactPoints[i];
				glm::vec3 r1 = contactPoint - world.shapes[objs.x].center;
				glm::vec3 r2 = contactPoint - world.shapes[objs.y].center;
				float momentOfInertia1 = kinem1.mass * glm::pow(2, 2); // first 2 -> radius
				float momentOfInertia2 = kinem2.mass * glm::pow(2, 2);
				auto angularPart1 = glm::cross((1 / momentOfInertia1) * (glm::cross(r1, coll.dir)), r1);
				auto angularPart2 = glm::cross((1 / momentOfInertia2) * (glm::cross(r2, coll.dir)), r2);
				float angularPart = glm::dot(angularPart1 + angularPart2, coll.dir);

				float resulting = (1 + coefficient) * glm::dot(kinem1.velocity - kinem2.velocity, coll.dir) / (1 / kinem1.mass + 1 / kinem2.mass /* + angularPart*/);
				float resultingA = (1 + coefficient) * glm::dot(kinem1.velocity - kinem2.velocity, coll.dir) / (1 / kinem1.mass + 1 / kinem2.mass + angularPart);

				float deltaV1 = -(resulting / kinem1.mass);
				float deltaV2 = (resulting / kinem2.mass);
				if (kinem1.isStatic)
				{
					deltaV2 -= deltaV1;
					deltaV1 = 0;
				}
				if (kinem2.isStatic)
				{
					deltaV1 -= deltaV2;
					deltaV2 = 0;
				}

				glm::vec3 deltaW1 = -(1 / momentOfInertia1) * glm::cross(r1, resultingA * coll.dir);
				glm::vec3 deltaW2 = (1 / momentOfInertia2) * glm::cross(r2, resultingA * coll.dir);
				kinem1.w += deltaW1;
				kinem2.w += deltaW2;
				if (coll.depth > 0 && glm::dot(coll.dir, kinem1.velocity - kinem2.velocity) <= 0)
				{
					kinem1.velocity = kinem1.velocity + deltaV1 * coll.dir;
					kinem2.velocity = kinem2.velocity + deltaV2 * coll.dir;
				}
				/*if (kinem1.isStatic)
				{
					glm::vec3 changeInVel = -(1 + coefficient) * glm::dot(kinem2.velocity, coll.dir) * -coll.dir;
					kinem2.velocity = kinem2.velocity + changeInVel;

					float vNextFrame = glm::dot(kinem2.forces * (delta * 1) / kinem2.mass, coll.dir);
					float vNow = glm::dot(changeInVel, coll.dir);
					if (glm::dot(vNextFrame, vNow) < 0 && glm::abs(vNextFrame) > glm::abs(vNow))
						kinem2.velocity -= glm::dot(kinem2.velocity, coll.dir) * coll.dir;
				}
				if (kinem2.isStatic)
				{
					glm::vec3 changeInVel = -(1 + coefficient) * glm::dot(kinem1.velocity, coll.dir) * coll.dir;
					kinem1.velocity = kinem1.velocity + changeInVel;

					float vNextFrame = glm::dot(kinem1.forces * (delta * 1) / kinem1.mass, coll.dir);
					float vNow = glm::dot(changeInVel, coll.dir);
					if (glm::dot(vNextFrame, vNow) < 0 && glm::abs(vNextFrame) > glm::abs(vNow))
						kinem1.velocity -= glm::dot(kinem1.velocity, coll.dir) * coll.dir;
				}
				else
				{
					float resulting = (1 + coefficient) * glm::dot(kinem1.velocity - kinem2.velocity, coll.dir) / (1 / kinem1.mass + 1 / kinem2.mass + angularPart);
					float deltaV1 = -(resulting / kinem1.mass);
					float deltaV2 = (resulting / kinem2.mass);
					glm::vec3 deltaW1 = -(1 / momentOfInertia1) * glm::cross(r1, resulting * coll.dir);
					glm::vec3 deltaW2 = -(1 / momentOfInertia2) * glm::cross(r2, resulting * coll.dir);
					glm::vec3 up1 = glm::normalize(deltaW1);
					glm::vec3 up2 = glm::normalize(deltaW2);
					float upA1 = glm::length(deltaW1);
					float upA2 = glm::length(deltaW2);

					kinem1.aVelUp += up1;
					kinem2.aVelUp += up2;
					kinem1.aVelSpeed += upA1;
					kinem2.aVelSpeed += upA2;

					kinem1.velocity = kinem1.velocity + deltaV1 * coll.dir;
					kinem2.velocity = kinem2.velocity + deltaV2 * coll.dir;
				}*/
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
			if (coll.depth >= 0)
			{
				glm::vec3 cf1 = coll.dir * kinem1.forces;
				glm::vec3 cf2 = -coll.dir * kinem2.forces;
				kinem1.forces -= cf1;
				kinem2.forces -= cf2;
			}
		}
	}
	void System::RotationUpdate(float delta)
	{
		for (size_t i = 0; i < world.kinems.size(); i++)
		{
			if (world.kinems[i].isStatic || glm::dot(world.kinems[i].w, world.kinems[i].w) == 0) continue;

			float angle = glm::length(world.kinems[i].w);
			glm::vec3 up = world.kinems[i].w * (1 / angle);
			world.kinems[i].quaternion = glm::rotate(world.kinems[i].quaternion, angle * delta, up);
		}
	}

	void System::SetColliders(std::span<Collider> colls)
	{
		if constexpr (Config::CollisionDetectionGPU)
		{
			((CollisionDetectionGPU*)collisionDetector)->SetColliders(colls);
		}
		else
		{
			world.ClearColliderData();
			for (size_t i = 0; i < colls.size(); i++)
			{
				world.AddCollider(colls[i]);
			}
		}
	}

	void System::SetModels(std::span<glm::vec3> translate, std::span<glm::quat> quaternions)
	{
		if constexpr (Config::CollisionDetectionGPU)
		{
			MetaLogger().Warning("Not implemented");
			//((CollisionDetectionGPU*)collisionDetector)->SetupPhase(models);
		}
		else
		{
			world.UpdateTransforms(translate, quaternions);
		}
	}

	void System::VelocityUpdate(float delta)
	{
		for (size_t i = 0; i < world.kinems.size(); i++)
		{
			if (world.kinems[i].isStatic) continue;
			auto& k = world.kinems[i];

			glm::vec3 acceleration = k.forces / k.mass;
			auto deltaV = acceleration * delta;
			glm::vec3 translate = k.velocity * delta + 0.5f * acceleration * delta * delta;

			k.velocity += deltaV;
			k.position += translate;

			k.forces = g * k.mass * glm::vec3(0, -1, 0);
		}
	}
}