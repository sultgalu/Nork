#include "../Common.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace Nork::Components {
	glm::mat4 Transform::TranslationMatrix()
	{
		return glm::translate(glm::identity<glm::mat4>(), position);
	}
	glm::mat4 Transform::RotationMatrix()
	{
		return glm::mat4_cast(quaternion);
	}
	glm::mat4 Transform::ScaleMatrix()
	{
		return glm::scale(glm::identity<glm::mat4>(), scale);
	}
	glm::mat4 Transform::TranslationRotationMatrix()
	{
		return TranslationMatrix() * RotationMatrix();
	}
	glm::mat4& Transform::RecalcModelMatrix()
	{
		return modelMatrix = TranslationMatrix() * RotationMatrix() * ScaleMatrix();
	}
	glm::mat4 Transform::LocalModelMatrix()
	{
		return glm::scale(glm::translate(glm::identity<glm::mat4>(), localPosition) * glm::mat4_cast(localQuaternion), localScale);
	}
	Transform::Tr Transform::GetDifferenceFromModelMatrix() const
	{
		Transform::Tr diff;
		glm::vec3 skew; glm::vec4 persp;
		glm::decompose(modelMatrix, diff.scale, diff.quaternion, diff.position, skew, persp);
		diff.quaternion = glm::normalize(glm::conjugate(diff.quaternion));
		auto asd = glm::normalize(localQuaternion);
		diff.position = localPosition - diff.position;
		diff.quaternion = asd * diff.quaternion;
		diff.scale = localScale / diff.scale;
		return diff;
	}
	void Transform::SetToDecomposed(const glm::mat4)
	{
		static glm::vec3 skew; glm::vec4 persp;
		glm::decompose(modelMatrix, scale, quaternion, position, skew, persp);
		quaternion = glm::conjugate(quaternion);
	}
	void Transform::UpdateChild(Transform& child)
	{
		auto diff = GetDifferenceFromModelMatrix();
		if (diff.position != glm::vec3(0))
		{
			Logger::Info("asd");
		}
		auto diffMat = glm::scale(glm::translate(glm::identity<glm::mat4>(), diff.position) * glm::mat4_cast(diff.quaternion), diff.scale);
		child.Trans(diffMat);
	}
}
