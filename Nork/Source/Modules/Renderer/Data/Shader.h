#pragma once

namespace Nork::Renderer
{
	class Shader
	{
	public:
		Shader(const GLuint program, const std::unordered_map<std::string_view, GLint> uniforms) : program(program), uniformLocations(uniforms) {}
		void Use();
		void SetMat4(std::string_view name, glm::mat4& value);
		void SetVec4(std::string_view name, glm::vec4& value);
		void SetVec3(std::string_view name, glm::vec3& value);
		void SetFloat(std::string_view name, float value);
		void SetInt(std::string_view name, int value);

		GLuint GetProgram() { return program; }
		auto GetUniformMap() { return uniformLocations; }
	private:
		GLuint program;
		std::unordered_map<std::string_view, GLint> uniformLocations;
	};
}

