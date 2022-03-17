#include "../Common.h"

namespace Nork::Components {
	Transform::Transform() 
		: position(glm::vec3(0)), scale(glm::vec3(1)), quaternion(glm::identity<glm::quat>()),
		changed(true)
	{}
	glm::vec3 Transform::Translate(const glm::vec3& translation)
	{
		position += translation;
		changed = true;
		return position;
	}
	glm::quat Transform::Rotate(const glm::vec3& axis, float angle)
	{
		quaternion = glm::rotate(quaternion, angle, axis);
		changed = true;
		return quaternion;
	}
	glm::vec3 Transform::Scaling(const glm::vec3& _scale)
	{
		scale += _scale;
		changed = true;
		return scale;
	}
	glm::mat4 Transform::TranslationMatrix()
	{
		return glm::translate(glm::identity<glm::mat4>(), position);;
	}
	glm::mat4 Transform::RotationMatrix() 
	{
		return glm::mat4_cast(quaternion);
	}
	glm::mat4 Transform::TranslationRotationMatrix() 
	{
		return TranslationMatrix() * RotationMatrix();
	}
	glm::mat4 Transform::ModelMatrix() 
	{
		if (changed)
		{
			modelMatrix = glm::scale(TranslationRotationMatrix(), scale);
			changed = false;
		}
		return modelMatrix;
	}
	Transform& Transform::SetPosition(const glm::vec3& v)
	{
		if (position != v)
		{
			position = v;
			changed = true;
		}
		return *this;
	}
	Transform& Transform::SetScale(const glm::vec3& v)
	{
		if (scale != v)
		{
			scale = v;
			changed = true;
		}
		return *this;
	}
	Transform& Transform::SetRotation(const glm::quat& v)
	{
		if (quaternion != v)
		{
			quaternion = v;
			changed = true;
		}
		return *this;
	}
}
