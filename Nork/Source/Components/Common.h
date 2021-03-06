#pragma once

namespace Nork::Components
{

	struct Transform
	{
		glm::mat4 modelMatrix = RecalcModelMatrix();

		glm::mat4 TranslationMatrix();
		glm::mat4 RotationMatrix();
		glm::mat4 TranslationRotationMatrix();
		glm::mat4& RecalcModelMatrix();
		glm::mat4 LocalModelMatrix();

		void Rotate(const glm::vec3& axis, float angle) { localQuaternion = glm::rotate(localQuaternion, angle, axis); }
		void SetRotation(const glm::vec3& axis, float angle) { localQuaternion = glm::angleAxis(angle, glm::normalize(axis)); }
		glm::vec3 RotationAxis() const { return glm::axis(quaternion); }
		float RotationAngle() const { return glm::angle(quaternion); }
		float RotationAngleDegrees() const { return glm::degrees(glm::angle(quaternion)); }

		const glm::vec3& Position() const { return position; }
		const glm::vec3& Scale() const { return scale; }
		const glm::quat& Quaternion() const { return quaternion; }

		void UpdateGlobalByParent(const Transform& parent)
		{
			auto mat = parent.modelMatrix * LocalModelMatrix();

			auto trans = glm::vec3(mat[0][3], mat[1][3], mat[2][3]);
			mat[0][3] = 0; mat[1][3] = 0; mat[2][3] = 0;

			position = trans;
			scale = localScale;
			quaternion = localQuaternion;
			// glm::tra
			modelMatrix = mat;
			//RecalcModelMatrix();
		}
		void UpdateGlobalWithoutParent()
		{
			position = localPosition;
			scale = localScale;
			quaternion = localQuaternion;
			RecalcModelMatrix();
		}
	private:
		glm::vec3 position = glm::vec3(0);
		glm::vec3 scale = glm::vec3(1);
		glm::quat quaternion = glm::identity<glm::quat>();
	public:
		glm::vec3 localPosition = glm::vec3(0);
		glm::vec3 localScale = glm::vec3(1);
		glm::quat localQuaternion = glm::identity<glm::quat>();
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

	struct Kinematic
	{
		float mass = 1;
		glm::vec3 velocity;
		glm::vec3 forces;
		glm::vec3 w = glm::vec3(0);
		glm::vec3 torque = glm::vec3(0);
		float I;
	};
}