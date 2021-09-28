#include "../Utils.h"

namespace Nork::Renderer::Utils::VAO
{
	void SetVaoAttribs(std::vector<int> attrLens)
	{
		int stride = 0;
		for (int i = 0; i < attrLens.size(); i++)
			stride += attrLens[i];
		stride *= sizeof(float);

		int offset = 0;
		for (int i = 0; i < attrLens.size(); i++)
		{
			glVertexAttribPointer(i, attrLens[i], GL_FLOAT, false, stride, (void*)(offset * sizeof(float)));
			glEnableVertexAttribArray(i);
			offset += attrLens[i];
		}
	}

	Builder::Builder()
	{
		glGenVertexArrays(1, &this->vao);
	}
	Builder& Builder::AddBuffer(unsigned int* handler, GLenum target, GLenum usage, int size, void* data)
	{
		glBindVertexArray(this->vao);

		glGenBuffers(1, handler);
		glBindBuffer(target, *handler);
		glBufferData(target, size, data, usage);

		return *this;
	}
	Builder& Builder::SetAttribs(std::vector<int> attrLens)
	{
		glBindVertexArray(this->vao);

		SetVaoAttribs(attrLens);

		return *this;
	}
	unsigned int Builder::GetVertexArrayBuffer()
	{
		return this->vao;
	}
}