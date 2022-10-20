module Nork.Renderer;

namespace Nork::Renderer {
	std::unordered_map<std::string, size_t> Shader::QueryAllUniformNamesAndTypes()
    {
		std::unordered_map<std::string, size_t> retval;
		int count = 0;
		glUseProgram(handle);
		glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &count);

		for (int i = 0; i < count; i++)
		{
			char buf[100];
			int nameLen = 0;
			int size = 0; // for arrays (that are not of structures)
			GLenum type;
			glGetActiveUniform(handle, i, sizeof(buf), &nameLen, &size, &type, buf);
			int loc = glGetUniformLocation(handle, buf);

			if (loc == -1)
			{
				Logger::Warning("Got uniform name, but couldn't read it's location: ", buf);
				continue;
			}

			switch (type)
			{
			case GL_FLOAT:
				retval[buf] = typeid(float).hash_code();
				break;
			case GL_FLOAT_VEC2:
				retval[buf] = typeid(glm::vec2).hash_code();
				break;
			case GL_FLOAT_VEC3:
				retval[buf] = typeid(glm::vec3).hash_code();
				break;
			case GL_FLOAT_VEC4:
				retval[buf] = typeid(glm::vec4).hash_code();
				break;
			case GL_INT:
				retval[buf] = typeid(int).hash_code();
				break;
			case GL_FLOAT_MAT4:
				retval[buf] = typeid(glm::mat4).hash_code();
				break;
			case GL_SAMPLER_2D:
			case GL_SAMPLER_CUBE:
			case GL_SAMPLER_2D_SHADOW:
			case GL_SAMPLER_CUBE_SHADOW:
				retval[buf] = typeid(int).hash_code();
				break;
			default:
				break;
			}
		}
		return retval;
    }
}
