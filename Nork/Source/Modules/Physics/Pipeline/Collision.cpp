#include "Collision.h"
#include "../Algorithms/Clip.h"
#include "../Algorithms/SAT.h"
#include "../Utils.h"

namespace Nork::Physics {
	static glm::vec3 GetIntersectionPoint(glm::vec3 p1, glm::vec3 p2, glm::vec3 v1, glm::vec3 v2)
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

	static std::vector<glm::vec3> FaceContactPoints(const Collider& facecollider, const Collider& vertcollider, const uint32_t faceIdx, const uint32_t vertIdx)
	{
		auto& faceNormal = facecollider.faces[faceIdx].norm;
		auto& faceVert = facecollider.verts[facecollider.faces[faceIdx].vertIdx];

		auto sideFaces = vertcollider.SideFacesOfVert(vertIdx);
		auto sideEdges = vertcollider.Edges(vertcollider.verts[vertIdx]);

		//auto faceEdgeIdxs = facecollider.EdgesOnFace(faceIdx);
		constexpr float faceBias = 0.0f;
		constexpr float edgeBias = 90.0f;
		for (auto& sideFaceIdx : sideFaces)
		{
			auto& sideFaceNorm = vertcollider.faces[sideFaceIdx].norm;
			auto cos = glm::dot(faceNormal, glm::normalize(-sideFaceNorm));
			auto degrees = glm::degrees(glm::acos(cos));
			if (glm::abs(degrees) <= faceBias)
			{
				std::vector<glm::vec3> facePoints;
				for (auto& vertIdx : vertcollider.faceVerts[sideFaceIdx])
				{
					facePoints.push_back(vertcollider.verts[vertIdx]);
				}
				return Clip::PointsOnFace(facecollider, faceIdx, facePoints);
			}
		}
		for (auto& sideEdgeIdx : sideEdges)
		{
			auto edgeVec = vertcollider.verts[sideEdgeIdx.first] - vertcollider.verts[sideEdgeIdx.second];
			auto dotProd = glm::abs(glm::dot(faceNormal, edgeVec));
			auto degrees = glm::degrees(glm::acos(dotProd));
			if (degrees >= edgeBias)
			{
				auto clipped = Clip::PointsOnFace(facecollider, faceIdx, { vertcollider.verts[sideEdgeIdx.first], vertcollider.verts[sideEdgeIdx.second] });
				if (!clipped.empty())
				{
					return clipped;
				}
			}
		}
		if (!Clip::PointsOnFace(facecollider, faceIdx, { vertcollider.verts[vertIdx] }).empty())
		{
			return { vertcollider.verts[vertIdx] };
		}
		for (auto& sideEdgeIdx : sideEdges)
		{
			auto clipped = Clip::PointsOnPlane(faceNormal, faceVert, { vertcollider.verts[sideEdgeIdx.first], vertcollider.verts[sideEdgeIdx.second] }); // max 1
			if (!clipped.empty())
			{
				return clipped;
			}
		}
		Logger::Error("");
		return {};
	}

	void Collision::_1NarrowPhase()
	{
		if (Kinem1().isStatic && Kinem2().isStatic)
			return;
		satRes = SAT(Collider1(), Collider2()).GetResult();
		satRes.depth *= -1;
		satRes.dir *= -1;
		isColliding = satRes.depth >= 0;

		if (satRes.type == CollisionType::VertFace)
		{
			std::swap(idx1, idx2);
			satRes.type = CollisionType::FaceVert;
			satRes.dir *= -1;
		}
	}
	void Collision::_2GenerateContactPoints()
	{
		if (!isColliding)
			return;
		if (Kinem1().isStatic && Kinem2().isStatic)
			return;

		constexpr float bias = 0.1f;
		if (satRes.type == CollisionType::FaceVert)
		{
			contactPoints = FaceContactPoints(Collider1(), Collider2(), satRes.featureIdx1, satRes.featureIdx2);
			contactCenter = Center(contactPoints);
			if (glm::isnan(contactCenter.x))
				Logger::Error("");
		}
		// else if (satRes.type == CollisionType::VertFace)
		// {
		// 	contactPoints = FaceContactPoints(Collider2(), Collider1(), satRes.featureIdx1, satRes.featureIdx2);
		// 	contactCenter = Center(contactPoints);
		// 	if (glm::isnan(contactCenter.x))
		// 		Logger::Error("");
		// }
		else if (satRes.type == CollisionType::EdgeEdge)
		{
			auto& edge1 = Collider1().edges[satRes.featureIdx1];
			auto& edge2 = Collider2().edges[satRes.featureIdx2];

			glm::vec3 edge1Dir = Collider1().EdgeDirection(edge1);
			glm::vec3 edge2Dir = Collider2().EdgeDirection(edge2);

			glm::vec3 edgePoint1 = Collider1().FirstVertFromEdge(edge1);
			glm::vec3 edgePoint2 = Collider2().FirstVertFromEdge(edge2);

			glm::vec3 planeNorm = satRes.dir;
			float sd = SignedDistance(planeNorm, edgePoint1, edgePoint2);
			glm::vec3 edgePoint2Translated = edgePoint2 - sd * planeNorm;

			contactPoints = { GetIntersectionPoint(edgePoint1, edgePoint2Translated, edge1Dir, edge2Dir) };
			contactCenter = contactPoints[0]; //+sd * 0.5f * planeNorm;
		}
	}
	void Collision::_3CalculateForces()
	{
		if (!isColliding)
			return;

		const auto& kinem1 = Kinem1();
		const auto& kinem2 = Kinem2();

		if (kinem1.isStatic && kinem2.isStatic)
			return;

		glm::vec3 t = glm::vec3(0);
		const glm::vec3& contactPoint = contactCenter;
		glm::vec3 r1 = contactPoint - Obj1().centerOfMass;
		glm::vec3 r2 = contactPoint - Obj2().centerOfMass;
		auto vel1 = kinem1.velocity + glm::cross(kinem1.w, r1);
		auto vel2 = kinem2.velocity + glm::cross(kinem2.w, r2);
		auto velDiffLenSquared = glm::dot(vel1 - vel2, vel1 - vel2);

		if (satRes.depth > 0)
		{
			auto angularPart1 = glm::cross((1 / kinem1.I) * (glm::cross(r1, satRes.dir)), r1);
			auto angularPart2 = glm::cross((1 / kinem2.I) * (glm::cross(r2, satRes.dir)), r2);
			constexpr auto vec0 = glm::zero<glm::vec3>();
			float angularPart = glm::dot((kinem1.isStatic ? vec0 : angularPart1) + (kinem2.isStatic ? vec0 : angularPart2), satRes.dir);

			t = vel1 - vel2;
			if (t != glm::zero<glm::vec3>())
			{
				t = glm::cross(satRes.dir, glm::normalize(t));
				t = glm::cross(t, satRes.dir);
			}

			float coefficient = velDiffLenSquared < 0.01f ? 0 : glm::min((kinem1.elasticity + kinem2.elasticity) / 2, 1.0f);
			float friction = glm::min((kinem1.friction + kinem2.friction) / 2.0f, 1.0f);
			float resulting = -(1 + coefficient) * glm::dot(vel1 - vel2, satRes.dir)
				/ ((kinem1.isStatic ? 0 : 1 / kinem1.mass) + (kinem2.isStatic ? 0 : 1 / kinem2.mass) + angularPart);
			glm::vec3 resultingVec = resulting * (satRes.dir -friction * t);

			this->deltaW1 = kinem1.isStatic ? vec0 : (1.0f / kinem1.I) * glm::cross(r1, resultingVec);
			this->deltaW2 = kinem2.isStatic ? vec0 : (-1.0f / kinem2.I) * glm::cross(r2, resultingVec);
			if (glm::isnan(this->deltaW1.x) || glm::isnan(this->deltaW2.x))
				Logger::Error("glm::isnan(this->deltaW1.x) || glm::isnan(this->deltaW2.x)");

			if (satRes.depth > 0 && glm::dot(satRes.dir, kinem1.velocity - kinem2.velocity) <= 0)
			{
				this->deltaV1 = kinem1.isStatic ? vec0 : (1.0f / kinem1.mass) * resultingVec;
				this->deltaV2 = kinem2.isStatic ? vec0 : (-1.0f / kinem2.mass) * resultingVec;
			}
			if (glm::isnan(kinem1.velocity.x) || glm::isnan(kinem2.velocity.x))
				Logger::Error("glm::isnan(kinem1.velocity.x) || glm::isnan(kinem2.velocity.x)");

			auto afterV1 = kinem1.velocity + this->deltaV1;
			auto afterV2 = kinem2.velocity + this->deltaV2;
			auto afterW1 = kinem1.w + this->deltaW1;
			auto afterW2 = kinem2.w + this->deltaW2;
			auto afterVel1 = afterV1 + glm::cross(afterW1, r1);
			auto afterVel2 = afterV2 + glm::cross(afterW2, r2);
			auto cr = -glm::dot(afterVel1 - afterVel2, satRes.dir) / glm::dot(vel1 - vel2, satRes.dir);
			auto diff = glm::abs(coefficient - cr);
		}
		if (satRes.depth > 0)
		{
			if (!kinem1.isStatic && kinem2.isStatic)
			{
				this->deltaP1 = satRes.dir * satRes.depth;
			}
			else if (kinem1.isStatic && !kinem2.isStatic)
			{
				this->deltaP2 = -satRes.dir * satRes.depth;
			}
			else if (!kinem1.isStatic && !kinem2.isStatic)
			{
				auto part1 = kinem2.mass;
				auto part2 = kinem1.mass;
				glm::vec3 translate = satRes.dir * satRes.depth / (part1 + part2);
				this->deltaP1 = translate * (part1);
				this->deltaP2 = -translate * (part2);
			}
		}
	}

	void Collision::_4ResolveVelocities()
	{
		GetWorld().objs[idx1.objIdx].kinem.velocity += this->deltaV1;
		GetWorld().objs[idx2.objIdx].kinem.velocity += this->deltaV2;
	}
	void Collision::_4ResolveAngularVelocities()
	{
		GetWorld().objs[idx1.objIdx].kinem.w += this->deltaW1;
		GetWorld().objs[idx2.objIdx].kinem.w += this->deltaW2;
	}
	void Collision::_4ResolvePositions()
	{
		GetWorld().objs[idx1.objIdx].kinem.position += this->deltaP1;
		GetWorld().objs[idx2.objIdx].kinem.position += this->deltaP2;
	}
	void Collision::_4ResolveAll()
	{
		if (!isColliding)
			return;

		GetWorld().objs[idx1.objIdx].kinem.velocity += this->deltaV1;
		GetWorld().objs[idx2.objIdx].kinem.velocity += this->deltaV2;

		GetWorld().objs[idx1.objIdx].kinem.w += this->deltaW1;
		GetWorld().objs[idx2.objIdx].kinem.w += this->deltaW2;

		GetWorld().objs[idx1.objIdx].kinem.position += this->deltaP1;
		GetWorld().objs[idx2.objIdx].kinem.position += this->deltaP2;
	}
}