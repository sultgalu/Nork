#pragma once

namespace Nork::Renderer {
	enum class DepthFunc
	{
		Less = GL_LESS, LessOrEqual = GL_LEQUAL
	};
	enum class CullFaceSide
	{
		Back = GL_BACK, Front = GL_FRONT, FrontAndBack = GL_FRONT_AND_BACK
	};
	enum class BlendFunc
	{
		None, SrcAlpha_1MinuseSrcAlpha
	};
	class Capabilities
	{
	public:
		struct Disables
		{
			Disables& DepthTest();
			Disables& CullFace();
			Disables& Blend();
		};
		struct Enables
		{
			Enables& DepthTest(DepthFunc func = DepthFunc::Less);
			Enables& CullFace(CullFaceSide side = CullFaceSide::Back);
			Enables& Blend(BlendFunc func = BlendFunc::SrcAlpha_1MinuseSrcAlpha);
			Disables Disable() { return Disables(); }
		};
		Enables Enable()
		{
			return Enables();
		}
		Disables Disable()
		{
			return Disables();
		}
	};
}