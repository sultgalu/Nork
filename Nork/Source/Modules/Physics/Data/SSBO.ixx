export module Nork.Physics:SSBO;

export namespace Nork::Physics
{
	template<class T>
	struct ShaderStorageBuffer
	{
		ShaderStorageBuffer(uint32_t idx, size_t size = 0, void* data = nullptr)
		{
			glGenBuffers(1, &id);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, idx, id);
		}
		~ShaderStorageBuffer()
		{
			glDeleteBuffers(1, &id);
		}
		void Data(std::span<T> data)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glBufferData(GL_SHADER_STORAGE_BUFFER, data.size_bytes(), data.data(), GL_DYNAMIC_DRAW);
			this->size = data.size_bytes();
		}

		void Data(std::vector<T>& data)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_DYNAMIC_DRAW);
			this->size = data.size() * sizeof(T);
		}

		void SetSize(size_t size)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
			this->size = size;
		}
		void GetData(std::span<T> dest)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dest.size_bytes(), dest.data());
		}
		void GetData(void* dest, uint32_t size)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, dest);
		}

		GLuint id;
		size_t size;
	};
}