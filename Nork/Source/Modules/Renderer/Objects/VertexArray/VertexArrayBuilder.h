#pragma once
#include "VertexArray.h"

namespace Nork::Renderer {

	class VertexArrayBuilder
	{
	public:
		VertexArrayBuilder& IBO(std::shared_ptr<Buffer> ibo)
		{
			this->ibo = ibo;
			return *this;
		}
		VertexArrayBuilder& VBO(std::shared_ptr<Buffer> vbo)
		{
			this->vbo = vbo;
			return *this;
		}
		VertexArrayBuilder& Attributes(const std::vector<int>& attrLens)
		{
			this->attrLens = attrLens;
			return *this;
		}
		std::shared_ptr<VertexArray> Create(std::source_location loc = std::source_location::current());
	private:
		void SetAttribs();
		void Validate()
		{
			if (attrLens.size() == 0)
			{
				std::abort();
			}
		}
	private:
		std::vector<int> attrLens;
		int stride;
		GLuint handle;
		std::shared_ptr<Buffer> vbo, ibo;
	};
}