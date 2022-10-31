#pragma once

#include "../../Objects/Texture/Texture.h"
#include "../../Objects/Framebuffer/LightFramebuffer.h"

namespace Nork::Renderer 
{
	class Stage
	{
	public:
		/// @param source: input, can be used as output
		/// @param destination: optional output
		/// @returns whether result was rendered to destination or not
		virtual bool Execute(Framebuffer& source, Framebuffer& destination) = 0;
	};
}