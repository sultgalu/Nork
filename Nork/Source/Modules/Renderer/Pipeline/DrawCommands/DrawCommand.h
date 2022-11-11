#pragma once

namespace Nork::Renderer {
	class DrawCommand
	{
	public:
		virtual void operator()() const = 0;
	};
}