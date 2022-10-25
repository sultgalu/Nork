#include "Collision.h"
#include "../Utils/Clip.h"
#include "../Utils/SAT.h"

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
		satRes = SAT(Collider1(), Collider2()).GetResult();
		satRes.depth *= -1;
		satRes.dir *= -1;
		isColliding = satRes.depth >= 0;
	}
	void Collision::_2GenerateContactPoints()
	{
		if (!isColliding)
			return;

		constexpr float bias = 0.1f;
		if (satRes.type == CollisionType::FaceVert)
		{
			contactPoints = FaceContactPoints(Collider1(), Collider2(), satRes.featureIdx1, satRes.featureIdx2);
			contactCenter = Center(contactPoints);
			if (glm::isnan(contactCenter.x))
				Logger::Error("");
		}
		else if (satRes.type == CollisionType::VertFace)
		{
			contactPoints = FaceContactPoints(Collider2(), Collider1(), satRes.featureIdx1, satRes.featureIdx2);
			contactCenter = Center(contactPoints);
			if (glm::isnan(contactCenter.x))
				Logger::Error("");
		}
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

		// if (kinem1.isStatic)
		// 	kinem1.mass = 100000000.0f;
		// if (kinem2.isStatic)
		// 	kinem2.mass = 100000000.0f;

		glm::vec3 t = glm::vec3(0);
		float fr = -0.9f;
		float fr2 = 1.0f;
		float bias = 0.1f;
		const glm::vec3& contactPoint = contactCenter;
		glm::vec3 r1 = contactPoint - Collider1().center;
		glm::vec3 r2 = contactPoint - Collider2().center;
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
		if (satRes.depth == 0)
		{
			satRes.depth = 0;
		}
		if (satRes.depth > 0)
		{
			float actualCoefficient = coefficient;
			if (velDiffLenSquared < 0.01f)
			{
				actualCoefficient = 0;
				// fr = 0.0f;
			}

			float resulting = (1 + actualCoefficient) * glm::dot(kinem1.velocity - kinem2.velocity, satRes.dir) / (1 / kinem1.mass + 1 / kinem2.mass /* + angularPart*/);

			auto angularPart1 = glm::cross((1 / kinem1.I) * (glm::cross(r1, satRes.dir)), r1);
			auto angularPart2 = glm::cross((1 / kinem2.I) * (glm::cross(r2, satRes.dir)), r2);

			float angularPart = glm::dot(angularPart1 + angularPart2, satRes.dir);

			t = vel1 - vel2;
			if (t != glm::vec3(0))
			{
				t = glm::cross(satRes.dir, glm::normalize(t));
				t = glm::cross(t, satRes.dir);
			}

			float resultingA = -(1 + actualCoefficient) * glm::dot(vel1 - vel2, satRes.dir) / (1 / kinem1.mass + 1 / kinem2.mass + angularPart);
			resulting = resultingA;
			glm::vec3 deltaW1 = (1.0f / kinem1.I + (kinem2.isStatic ? 1.0f / kinem2.I : 0)) * glm::cross(r1, resultingA * (satRes.dir + fr * t));
			glm::vec3 deltaW2 = -(1.0f / kinem2.I + (kinem1.isStatic ? 1.0f / kinem1.I : 0)) * glm::cross(r2, resultingA * (satRes.dir + fr * t));
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
			this->deltaW1 = deltaW1;
			this->deltaW2 = deltaW2;

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
			if (satRes.depth > 0 && glm::dot(satRes.dir, kinem1.velocity - kinem2.velocity) <= 0)
			{
				this->deltaV1 = resulting * deltaV1 * (satRes.dir + fr * t);
				this->deltaV2 = resulting * deltaV2 * (satRes.dir + fr * t);
			}

			if (glm::isnan(kinem1.velocity.x) || glm::isnan(kinem2.velocity.x))
				Logger::Error("");

			auto newVel1 = kinem1.velocity + glm::cross(kinem1.w, r1);
			auto newVel2 = kinem2.velocity + glm::cross(kinem2.w, r2);

			auto cr = -glm::dot((newVel1 - newVel2), satRes.dir) / glm::dot((vel1 - vel2), satRes.dir);
			if (cr == actualCoefficient)
			{
				actualCoefficient = 0;
			}
			else
			{
				actualCoefficient = 0;
			}
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
				glm::vec3 translate = satRes.dir * (satRes.depth / 2);
				this->deltaP1 = translate;
				this->deltaP2 = -translate;
			}
		}
	}

	void Collision::_4ResolveVelocities()
	{
		GetWorld().kinems[obj1Idx].velocity += this->deltaV1;
		GetWorld().kinems[obj2Idx].velocity += this->deltaV2;
	}
	void Collision::_4ResolveAngularVelocities()
	{
		GetWorld().kinems[obj1Idx].w += this->deltaW1;
		GetWorld().kinems[obj2Idx].w += this->deltaW2;
	}
	void Collision::_4ResolvePositions()
	{
		GetWorld().kinems[obj1Idx].position += this->deltaP1;
		GetWorld().kinems[obj2Idx].position += this->deltaP2;
	}
	void Collision::_4ResolveAll()
	{
		if (!isColliding)
			return;

		GetWorld().kinems[obj1Idx].velocity += this->deltaV1;
		GetWorld().kinems[obj2Idx].velocity += this->deltaV2;

		GetWorld().kinems[obj1Idx].w += this->deltaW1;
		GetWorld().kinems[obj2Idx].w += this->deltaW2;

		GetWorld().kinems[obj1Idx].position += this->deltaP1;
		GetWorld().kinems[obj2Idx].position += this->deltaP2;
	}
}