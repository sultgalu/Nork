#pragma once

namespace Nork::Components
{
	struct Transform
	{
		Transform() : position(glm::vec3(0)), scale(glm::vec3(1)), rotation(glm::vec3(0)) {}
		glm::vec3 position, scale, rotation;
		mutable glm::mat4 modelMatrix = GetModelMatrix();
		bool changed = true;
		glm::mat4 GetModelMatrix() const
		{
			if (changed)
			{
				glm::mat4 rot = glm::rotate(glm::identity<glm::mat4>(), rotation.z, glm::vec3(0, 0, 1));
				rot *= glm::rotate(glm::identity<glm::mat4>(), rotation.x, glm::vec3(1, 0, 0));
				rot *= glm::rotate(glm::identity<glm::mat4>(), rotation.y, glm::vec3(0, 1, 0)); // rotation around y-axis stays local (most used rotation usually)
				modelMatrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), position) * rot, scale);
				// changed = false;
			}
			return modelMatrix;
		}
	};

	struct Tag
	{
		std::string tag = "#tbd";
	};

}