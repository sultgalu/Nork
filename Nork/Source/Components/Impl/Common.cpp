#include "../Common.h"

namespace Nork::Components {
	glm::mat4 Transform::TranslationMatrix()
	{
		return glm::translate(glm::identity<glm::mat4>(), position);
	}
	glm::mat4 Transform::RotationMatrix() 
	{
		return glm::mat4_cast(quaternion);
	}
	glm::mat4 Transform::TranslationRotationMatrix() 
	{
		return TranslationMatrix() * RotationMatrix();
	}
	glm::mat4& Transform::RecalcModelMatrix()
	{
		return modelMatrix = glm::scale(TranslationRotationMatrix(), scale);
	}
}
