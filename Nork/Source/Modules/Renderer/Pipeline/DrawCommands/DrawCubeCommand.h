#pragma once

#include "../../Objects/VertexArray/VertexArray.h"
#include "DrawCommand.h"

namespace Nork::Renderer {
	class DrawCubeCommand : public DrawCommand
	{
	public:
		DrawCubeCommand();

		std::shared_ptr<VertexArray> vao;
		void operator()() const override;
	};
}