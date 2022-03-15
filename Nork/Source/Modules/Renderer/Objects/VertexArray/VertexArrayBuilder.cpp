#include "VertexArrayBuilder.h"
#include "../GLManager.h"

namespace Nork::Renderer {
	std::shared_ptr<VertexArray> VertexArrayBuilder::Create()
	{
		Logger::Info("Creating vertex array ", handle, ".");
		Validate();
		glGenVertexArrays(1, &handle);
		SetAttribs();
		auto vao = std::make_shared<VertexArray>(handle, attrLens, stride, vbo, ibo);
		GLManager::Get().vaos[vao->GetHandle()] = vao;
		return vao;
	}
	void VertexArrayBuilder::SetAttribs()
	{
		this->attrLens = attrLens;
		glBindVertexArray(handle);
		vbo->Bind(BufferTarget::Vertex);
		if (ibo != nullptr)
		{
			ibo->Bind(BufferTarget::Index);
		}

		stride = 0;
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
}
