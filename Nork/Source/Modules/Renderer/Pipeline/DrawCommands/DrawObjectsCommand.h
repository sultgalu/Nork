#pragma once

#include "../../Objects/VertexArray/VertexArray.h"
#include "../../Model/Object.h"
#include "DrawCommand.h"

namespace Nork::Renderer {
	class DrawObjectsCommand : public DrawCommand
	{
	public:
		DrawObjectsCommand() = default;
		DrawObjectsCommand(std::shared_ptr<VertexArray> vao, std::span<Object> objects);

		std::shared_ptr<VertexArray> vao;
		std::shared_ptr<Buffer> materialModelIndices;
		std::vector<VertexArray::DrawElementsIndirectCommand> indirects;
		void operator()() const override;
	};
}