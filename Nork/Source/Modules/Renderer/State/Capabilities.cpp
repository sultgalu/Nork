#include "Capabilities.h"

namespace Nork::Renderer {
	static DepthFunc depthFunc;
	static bool depthTest;
	static CullFaceSide cullFaceSide;
	static bool cullFace;
	static BlendFunc blendFunc;
	static bool blend;

	static std::array<GLenum, 2> GetSrcAndDst(BlendFunc func)
	{
		using enum BlendFunc;
		switch (func)
		{
		case SrcAlpha_1MinuseSrcAlpha:
			return { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };
		default:
			return { GL_NONE ,GL_NONE };
		}
	}

	Capabilities::Enables& Capabilities::Enables::DepthTest(DepthFunc func)
	{
		if (!depthTest)
		{
			glEnable(GL_DEPTH_TEST);
			depthTest = true;
		}
		if (depthFunc != func)
		{
			glDepthFunc(static_cast<GLenum>(func));
			depthFunc = func;
		}
		return *this;
	}
	Capabilities::Enables& Capabilities::Enables::CullFace(CullFaceSide side)
	{
		if (!cullFace)
		{
			glEnable(GL_CULL_FACE);
			cullFace = true;
		}
		if (cullFaceSide != side)
		{
			glCullFace(static_cast<GLenum>(side));
			cullFaceSide = side;
		}
		return *this;
	}
	Capabilities::Enables& Capabilities::Enables::Blend(BlendFunc func)
	{
		if (!blend)
		{
			glEnable(GL_BLEND);
			blend = true;
		}
		if (blendFunc != func)
		{
			auto srcAndDest = GetSrcAndDst(func);
			glBlendFunc(srcAndDest[0], srcAndDest[1]);
			blendFunc = func;
		}
		return *this;
	}
	Capabilities::Disables& Capabilities::Disables::Blend()
	{
		if (blend)
		{
			glDisable(GL_BLEND);
			blend = false;
		}
		return *this;
	}
	Capabilities::Disables& Capabilities::Disables::DepthTest()
	{
		if (depthTest)
		{
			glDisable(GL_DEPTH_TEST);
			depthTest = false;
		}
		return *this;
	}
	Capabilities::Disables& Capabilities::Disables::CullFace()
	{
		if (cullFace)
		{
			glDisable(GL_CULL_FACE);
			cullFace = false;
		}
		return *this;
	}
}