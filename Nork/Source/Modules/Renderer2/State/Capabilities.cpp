#include "Capabilities.h"

namespace Nork::Renderer {
	DepthTestCap& Capabilities::DepthTest()
	{
		static DepthTestCap cap;
		return cap;
	}
	CullFaceCap& Capabilities::CullFace()
	{
		static CullFaceCap cap;
		return cap;
	}
	BlendCap& Capabilities::Blend()
	{
		static BlendCap cap;
		return cap;
	}
}