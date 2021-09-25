#include "pch.h"
#include "Shader.h"
#include "../Config.h"

namespace Nork::Renderer{

	void Shader::Use()
	{
		glUseProgram(this->program);
	}
	void Shader::SetMat4(std::string_view name, glm::mat4& value)
	{
		glUniformMatrix4fv(this->uniformLocations[name], 1, false, (const GLfloat*)(&value));
	}
	void Shader::SetVec4(std::string_view name, glm::vec4& value)
	{
		glUniform4f(this->uniformLocations[name], value.x, value.y, value.z, value.w);
	}
	void Shader::SetVec3(std::string_view name, glm::vec3& value)
	{
		glUniform3f(this->uniformLocations[name], value.x, value.y, value.z);
	}
	void Shader::SetFloat(std::string_view name, float value)
	{
		glUniform1f(this->uniformLocations[name], value);
	}
	void Shader::SetInt(std::string_view name, int value)
	{
		glUniform1i(this->uniformLocations[name], value);
	}
}