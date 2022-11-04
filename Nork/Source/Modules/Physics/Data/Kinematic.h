#pragma once

namespace Nork::Physics
{
	struct KinematicData
	{
		glm::vec3 position = glm::zero<glm::vec3>();
		glm::vec3 velocity = glm::zero<glm::vec3>();
		glm::vec3 forces = glm::zero<glm::vec3>();
		float mass = 1;

		glm::quat quaternion = glm::quat(0, 0, 0, 1);
		glm::vec3 w = glm::zero<glm::vec3>();
		glm::vec3 torque = glm::zero<glm::vec3>();
		float I = 1;

		bool isStatic = false;
		bool applyGravity = true;
		float elasticity = 0.0f;
		float friction = 1.0f;
	};
}