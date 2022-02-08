#include "Capabilities.h"
#include "..\..\Renderer2\State\Capabilities.h"

namespace Nork::Renderer2
{
	DepthTestCap Capabilities::DepthTest()
	{
		static DepthTestCap cap;
		return cap;
	}
	
	CullFaceCap Capabilities::CullFace()
	{
		static CullFaceCap cap;
		return cap;
	}

	BlendCap Capabilities::Blend()
	{
		static BlendCap cap;
		return cap;
	}
}

