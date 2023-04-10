#include "../Common.h"

namespace Nork::Components {
	glm::mat4 Transform::TranslationMatrix() const
	{
		return glm::translate(glm::identity<glm::mat4>(), position);
	}
	glm::mat4 Transform::RotationMatrix() const
	{
		return glm::mat4_cast(quaternion);
	}
	glm::mat4 Transform::ScaleMatrix() const
	{
		return glm::scale(glm::identity<glm::mat4>(), scale);
	}
	glm::mat4 Transform::TranslationRotationMatrix() const
	{
		return TranslationMatrix() * RotationMatrix();
	}
	glm::mat4& Transform::RecalcModelMatrix()
	{
		return modelMatrix = TranslationMatrix() * RotationMatrix() * ScaleMatrix();
	}
	glm::mat4 Transform::LocalModelMatrix() const
	{
		return glm::scale(glm::translate(glm::identity<glm::mat4>(), localPosition) * glm::mat4_cast(localQuaternion), localScale);
	}
	/*Transform::Tr Transform::GetDifferenceFromModelMatrix() const
	{
		Transform::Tr diff;
		glm::vec3 skew; glm::vec4 persp;
		glm::decompose(modelMatrix, diff.scale, diff.quaternion, diff.position, skew, persp);
		diff.quaternion = glm::normalize(glm::conjugate(diff.quaternion));
		auto locQuat = glm::normalize(localQuaternion);
		diff.position = localPosition - diff.position;
		diff.quaternion = locQuat * diff.quaternion;
		diff.scale = localScale / diff.scale;
		return diff;
	}*/
	void Transform::SetToDecomposed(const glm::mat4& mat)
	{
		modelMatrix = mat;
		static glm::vec3 skew; glm::vec4 persp;
		glm::decompose(modelMatrix, scale, quaternion, position, skew, persp);
		quaternion = glm::conjugate(quaternion);
	}
}
