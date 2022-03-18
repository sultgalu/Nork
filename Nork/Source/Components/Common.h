#pragma once

namespace Nork::Components
{

	struct Transform
	{
		Transform();

		glm::vec3 Translate(const glm::vec3& translation);
		glm::vec3 Scaling(const glm::vec3& scale);
		glm::quat Rotate(const glm::vec3& axis, float angle);

		glm::mat4 TranslationMatrix();
		glm::mat4 RotationMatrix();
		glm::mat4 TranslationRotationMatrix();
		glm::mat4 ModelMatrix();

		const glm::vec3& GetPosition() const { return position; }
		const glm::vec3& GetScale() const { return scale; }
		const glm::quat& GetRotation() const { return quaternion; }
		Transform& SetPosition(const glm::vec3& v);
		Transform& SetScale(const glm::vec3& v);
		Transform& SetRotation(const glm::quat& v);


	private:
		glm::vec3 position, scale;
		glm::quat quaternion;
		
		bool changed;

		glm::mat4 modelMatrix;
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
		glm::vec3 w;
	};
}