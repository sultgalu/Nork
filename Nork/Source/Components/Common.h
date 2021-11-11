#pragma once

namespace Nork::Components
{
	struct Transform
	{
		Transform() : position(glm::vec3(0)), scale(glm::vec3(1)) {}
		glm::vec3 position, scale;
		glm::quat quaternion = glm::identity<glm::quat>();
		mutable glm::mat4 modelMatrix = GetModelMatrix();
		bool changed = true;
		glm::mat4 GetModelMatrix() const
		{
			if (changed)
			{
				modelMatrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), position) * RotationMatrix(), scale);
				// changed = false;
			}
			return modelMatrix;
		}

		glm::mat4 RotationMatrix() const
		{
			/*glm::mat4 rot = glm::rotate(glm::identity<glm::mat4>(), rotation.z, glm::vec3(0, 0, 1));
			rot *= glm::rotate(glm::identity<glm::mat4>(), rotation.x, glm::vec3(1, 0, 0));
			rot *= glm::rotate(glm::identity<glm::mat4>(), rotation.y, glm::vec3(0, 1, 0));*/
			return glm::mat4_cast(quaternion);
		}

		void Rotate(glm::vec3 axis, float angle)
		{
			quaternion = glm::rotate(quaternion, angle, axis);
		}

		glm::mat4 TranslationRotationMatrix() const
		{
			if (changed)
			{
				return glm::translate(glm::identity<glm::mat4>(), position) * RotationMatrix();
			}
		}
	};

	struct Tag
	{
		std::string tag = "#tbd";
	};

	struct Vertex
	{
		union
		{
			glm::vec3 pos;
			glm::vec3 position;
		};
		float selected = 0;
		uint32_t id;
		inline static uint32_t idCounter = 0;
		Vertex()
		{
			this->pos = glm::vec3(0);
			this->selected = false;
			this->id = ++idCounter;
		}
		Vertex(glm::vec3 pos, bool selected = false)
		{
			this->pos = pos;
			this->selected = selected;
			this->id = ++idCounter;
		}
	};

	struct OldKinematic
	{
		float mass = 1.0f;
		glm::vec3 velocity;
	};

	struct Kinematic
	{
		float mass = 1;
		glm::vec3 velocity;
		glm::vec3 forces;
		glm::vec3 w;
	};
}