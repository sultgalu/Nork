#include "PhysicsSystem.h"

namespace Nork::Physics
{
	void System::Update(World& world, float delta)
	{
		DetectCollisions(world);
		GenContactPoints(world);
		ResolveCollisions(world, delta);

		GenClipContactPoints(world);
		VelocityUpdate(world, delta); // can't put front couse it won't change Shape's verts pos.
		RotationUpdate(world, delta);
	}

	void System::GenClipContactPoints(World& world)
	{
		clipContactPoints.clear();
		if (!clip) return;

		for (size_t i = 0; i < world.shapes.size(); i++)
		{
			for (size_t j = i + 1; j < world.shapes.size(); j++)
			{
				auto res = Clip(world.shapes[i], world.shapes[j]).GetAllContactPointsBF();
				clipContactPoints.insert(clipContactPoints.end(), res.begin(), res.end());
			}
		}
	}

	void System::DetectCollisions(World& world)
	{
		detectionResults.clear();
		if (!detectCollisions) return;
		std::vector<uint32_t> aabbRes;
		if (aabb)
		{
			aabbRes = AABBTest::GetResult(world);
		}
		else
		{
			aabbRes = AABBTest::GetResult2(world);
		}
		for (size_t k = 0; k < aabbRes.size(); k++)
		{
			uint32_t i = k / world.shapes.size();
			uint32_t j = k % world.shapes.size();
			if (j == 0)
			{
				k += i; // (+ 1 by loop) skipping ones already checked (and identity)
				continue;
			}
			if (aabbRes[k] == 3)
			{
				if (gjk)
				{
					auto res = GJK(world.shapes[i].verts, world.shapes[j].verts).GetResult();
					if (res.has_value())
					{
						if (glm::dot(res.value().second, world.shapes[i].center - world.shapes[j].center) < 0)
							res.value().second *= -1;
						detectionResults.push_back(std::pair(std::pair(i, j), DetectionResult{
							.dir = res.value().second,
							.depth = res.value().first,
							}));
					}
				}
				else
				{
					auto res = SAT(world.shapes[i], world.shapes[j]).GetResult();
					detectionResults.push_back(std::pair(std::pair(i, j), DetectionResult{
						.dir = res.direction,
						.depth = res.distance,
						}));
				}
			}
		}
	}

	void System::GenContactPoints(World& world)
	{
		//contactPoints.clear();
		//if (!genContactPoints) return;

		//for (size_t i = 0; i < detectionResults.size(); i++)
		//{
		//	auto& res = detectionResults[i].second;
		//	//if (res.distance <= 0) continue;
		//	auto shape1 = world.shapes[detectionResults[i].first.first];
		//	auto shape2 = world.shapes[detectionResults[i].first.second];

		//	if (res.resType == res.FaceAndVert)
		//	{
		//		auto& face1 = res.faceAndVert.first;
		//		auto& vert2 = res.faceAndVert.second;
		//		
		//		auto norm = shape1.FaceNormal(face1);
		//		glm::vec3 contactPoint = vert2 + -res.direction * res.distance;
		//		contactPoints.push_back(contactPoint);
		//	}
		//	else if (res.resType == res.VertAndFace)
		//	{
		//		auto& vert1 = res.vertAndFace.first;
		//		auto& face2 = res.vertAndFace.second;

		//		auto norm = shape1.FaceNormal(face2);
		//		glm::vec3 contactPoint = vert1 + res.direction * res.distance;
		//		contactPoints.push_back(contactPoint);
		//	}
		//	else if (res.resType == res.EdgeAndEdge)
		//	{
		//		auto& edge1 = res.edgeAndEdge.first;
		//		auto& edge2 = res.edgeAndEdge.second;
		//		contactPoints.push_back(0.5f * (shape1.FirstVertFromEdge(edge1) - shape1.SecondVertFromEdge(edge1)));
		//		// TODO::
		//	}
		//}
	}
	void System::ResolveCollisions2(World& world)
	{
		if (!resolveCollisions) return;
		for (size_t i = 0; i < detectionResults.size(); i++)
		{
			auto& coll = detectionResults[i].second;
			if (coll.depth < -0.0001f)
				continue;
			auto& objs = detectionResults[i].first;
			auto& kinem1 = world.kinems[objs.first];
			auto& kinem2 = world.kinems[objs.second];

			if (resolveAngularMomentum && coll.depth > 0)
			{
				glm::vec3 r1 = contactPoints[i] - world.shapes[objs.first].center;
				glm::vec3 r2 = contactPoints[i] - world.shapes[objs.second].center;

				// should add, not set

				if (!kinem1.isStatic && kinem2.isStatic)
				{
					auto dir = kinem2.velocity - kinem1.velocity;
					kinem1.aVelUp += glm::cross(dir, r1);
					kinem1.aVelSpeed = glm::length(kinem1.aVelUp) * 1 / kinem1.mass * 2;
				}
				else if (kinem1.isStatic && !kinem2.isStatic)
				{
					auto dir = kinem2.velocity - kinem1.velocity;
					kinem2.aVelUp = -glm::cross(dir, r2);
					kinem2.aVelSpeed = glm::length(dir) * 1 / kinem2.mass * 2;
				}
				else if (!kinem1.isStatic && !kinem2.isStatic)
				{
					auto dir = kinem2.velocity - kinem1.velocity;
					kinem1.aVelUp = glm::cross(dir, r1);
					kinem2.aVelUp = glm::cross(dir, r2);
					kinem1.aVelSpeed = glm::length(dir) * kinem2.mass / kinem1.mass;
					kinem2.aVelSpeed = glm::length(dir) * kinem1.mass / kinem2.mass;
				}
			}

			if (resolveMomentum)
			{
				glm::vec3 momentum1 = kinem1.mass * kinem1.velocity;
				glm::vec3 momentum2 = kinem2.mass * kinem2.velocity;

				float collMom1 = glm::dot(momentum1, coll.dir); // get the amount of momentum in the direction of the counterforce
				float collMom2 = glm::dot(momentum2, coll.dir);

				if (collMom1 * collMom2 > 0)
				{
					//Logger::Warning("They shouldn't collide then");
				}

				// momentum1 = perpMom1 + collMom1 * coll.direction
				glm::vec3 perpV1 = kinem1.velocity - glm::dot(kinem1.velocity, coll.dir) * coll.dir; // for reconstructing the vector
				glm::vec3 perpV2 = kinem2.velocity - glm::dot(kinem2.velocity, coll.dir) * coll.dir;

				float graFrict = glm::dot(coll.dir, glm::vec3(0, -1, 0)) * g;
				float friction = glm::abs(collMom1) + glm::abs(collMom2); // +graFrict;

				/*if (glm::dot(perpV1, perpV1) < friction * friction)
					perpV1 = glm::vec3(0);
				else
					perpV1 -= friction * glm::normalize(perpV1);
				if (glm::dot(perpV2, perpV2) < friction * friction)
					perpV2 = glm::vec3(0);
				else
					perpV2 -= friction * glm::normalize(perpV2);

				*/

				glm::vec3 normPerp1 = glm::normalize(perpV1);
				glm::vec3 normPerp2 = glm::normalize(perpV2);
				if (!kinem1.isStatic && kinem2.isStatic)
				{
					/*glm::vec3 counter = 2 * friction * normPerp1;
					if (glm::dot(perpV1, perpV1) - glm::dot(perpV2, perpV2) > glm::dot(counter, counter))
					{
						perpV1 -= counter;
					}
					else
						perpV1 = glm::vec3(0);*/
					kinem1.velocity = perpV1; // kinem1.velocity - kinem1.velocity * coll.direction;
				}
				else if (kinem1.isStatic && !kinem2.isStatic)
				{
					/*glm::vec3 counter = 2 * friction * normPerp1;
					if (glm::dot(perpV1, perpV1) - glm::dot(perpV2, perpV2) > glm::dot(counter, counter))
					{
						perpV2 -= counter;
					}
					else
						perpV1 = glm::vec3(0);*/
					kinem2.velocity = perpV2; // kinem2.velocity - kinem2.velocity * coll.direction;
				}
				else if (!kinem1.isStatic && !kinem2.isStatic)
				{
					if (glm::dot(perpV1, perpV1) - glm::dot(perpV2, perpV2) >= glm::dot(friction * normPerp1, friction * normPerp2))
					{
						perpV1 -= friction * normPerp1;
						perpV2 -= friction * normPerp2;
					}
					float resultingV = (collMom1 + collMom2) / (kinem1.mass + kinem2.mass); // conservation of momentum

					kinem1.velocity = perpV1 +
						(coll.depth <= 0 ? glm::dot(kinem1.velocity, coll.dir) * coll.dir : resultingV * coll.dir); // collMom1 -> resulting (change in velocity in the direction of the counterforce)
					kinem2.velocity = perpV2 +
						(coll.depth <= 0 ? glm::dot(kinem1.velocity, coll.dir) * coll.dir : resultingV * coll.dir);
				}
			}
			if (resolvePositions)
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
	void System::ResolveCollisions(World& world, float delta)
	{
		if (!resolveCollisions) return;
		for (size_t i = 0; i < detectionResults.size(); i++)
		{
			auto& coll = detectionResults[i].second;
			if (coll.depth < -0.0001f)
				continue;
			auto& objs = detectionResults[i].first;
			auto& kinem1 = world.kinems[objs.first];
			auto& kinem2 = world.kinems[objs.second];

			if (resolveMomentum && coll.depth > 0)
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
			if (resolvePositions)
			{
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
			if (applyCounterForces)
			{
				if (coll.depth >= 0)
				{
					glm::vec3 cf1 = coll.dir * kinem1.forces;
					glm::vec3 cf2 = -coll.dir * kinem2.forces;
					kinem1.forces -= cf1;
					kinem2.forces -= cf2;
				}
			}
		}
	}

	void System::RotationUpdate(World& world, float delta)
	{
		if (!updateRotation) return;
		for (size_t i = 0; i < world.kinems.size(); i++)
		{
			if (world.kinems[i].isStatic || world.kinems[i].aVelUp == glm::zero<glm::vec3>()
				|| world.kinems[i].aVelSpeed == 0) continue;
			
			world.kinems[i].quaternion = glm::rotate(world.kinems[i].quaternion, world.kinems[i].aVelSpeed * delta, world.kinems[i].aVelUp);
		}
	}

	void System::VelocityUpdate(World& world, float delta)
	{
		if (!updateVelocities) return;
		for (size_t i = 0; i < world.kinems.size(); i++)
		{
			if (world.kinems[i].isStatic) continue;
			auto& k = world.kinems[i];

			glm::vec3 acceleration = k.forces / k.mass;
			auto deltaV = acceleration * delta;
			glm::vec3 translate = k.velocity * delta + 0.5f * acceleration * delta * delta;

			k.velocity += deltaV;
			k.position += translate;

			if (applyForces)
			{
				k.forces = g * k.mass * glm::vec3(0, -1, 0);
			}
		}
	}
}