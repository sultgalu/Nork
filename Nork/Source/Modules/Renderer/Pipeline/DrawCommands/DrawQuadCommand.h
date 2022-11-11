#pragma once

#include "../../Objects/VertexArray/VertexArray.h"
#include "DrawCommand.h"

namespace Nork::Renderer {
	class DrawQuadCommand : public DrawCommand
	{
	public:
		DrawQuadCommand();

		std::shared_ptr<VertexArray> vao;
		void operator()() const override;
	};
}