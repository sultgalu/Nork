#pragma once
#include "../GLObject.h"

namespace Nork::Renderer {
	enum class ShaderType : GLenum
	{
		None = GL_NONE,
		Vertex = GL_VERTEX_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
		Compute = GL_COMPUTE_SHADER,
	};

	class Shader : public GLObject
	{
	public:
		Shader(GLuint handle, std::vector<ShaderType> types)
			: GLObject(handle), shaderTypes(types)
		{}
		~Shader()
		{
			Logger::Info("Deleting shader ", handle, ".");
			glDeleteProgram(handle);
		}
		Shader& Use()
		{
			glUseProgram(handle);
			return *this;
		}
		Shader& SetMat4(const char* name, const glm::mat4& value)
		{
			glUniformMatrix4fv(GetOrQueryUniformLocation(name), 1, false, (const GLfloat*)(&value));
			return *this;
		}
		Shader& SetVec4(const char* name, const glm::vec4& value)
		{
			glUniform4f(GetOrQueryUniformLocation(name), value.x, value.y, value.z, value.w);
			return *this;
		}
		Shader& SetVec2(const char* name, const glm::vec2& value)
		{
			glUniform2f(GetOrQueryUniformLocation(name), value.x, value.y);
			return *this;
		}
		Shader& SetVec3(const char* name, const glm::vec3& value)
		{
			glUniform3f(GetOrQueryUniformLocation(name), value.x, value.y, value.z);
			return *this;
		}
		Shader& SetFloat(const char* name, float value)
		{
			glUniform1f(GetOrQueryUniformLocation(name), value);
			return *this;
		}
		Shader& SetInt(const char* name, int value)
		{
			glUniform1i(GetOrQueryUniformLocation(name), value);
			return *this;
		}
		GLint GetOrQueryUniformLocation(const char* name)
		{
			auto loc = uniformLocations.find(name);
			if (loc == uniformLocations.end())
			{
				return QueryAndSetUniformLocation(name);
			}
			return loc->second;
		}
		GLint QueryAndSetUniformLocation(const char* name)
		{
			auto loc = glGetUniformLocation(handle, name);
			if (loc == -1)
			{
				Logger::Error("Couldn't find uniform location for ", name);
			}
			uniformLocations[name] = loc;
			return loc;
		}
	public:
		const std::vector<ShaderType> shaderTypes;
	private:
		std::unordered_map<std::string, GLint> uniformLocations;
	private:
		GLenum GetIdentifier() override
		{
			return GL_PROGRAM;
		}
	};
}

