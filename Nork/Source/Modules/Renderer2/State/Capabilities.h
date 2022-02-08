#pragma once

namespace Nork::Renderer2 {
	
	template<GLenum Cap>
	class Capability
	{
	public:
		void Enable()
		{
			if (!isEnabled)
			{
				glEnable(Cap);
				isEnabled = true;
			}
		}
		void Disable()
		{
			if (isEnabled)
			{
				glDisable(Cap);
				isEnabled = false;
			}
		}
	protected:
		bool isEnabled = false;
	};

	class DepthTestCap : public Capability<GL_DEPTH_TEST>
	{
	public:
		enum class Func
		{
			Less = GL_LESS
		};

		void SetFunc(Func func)
		{
			if (currentFunc != func)
			{
				glDepthFunc(static_cast<GLenum>(func));
				currentFunc = func;
			}
		}
	private:
		Func currentFunc = Func::Less;
	};

	class CullFaceCap : public Capability<GL_CULL_FACE>
	{
	public:
		enum class Face
		{
			Back = GL_BACK, Front = GL_FRONT, FrontAndBack = GL_FRONT_AND_BACK
		};
		void SetFace(Face face)
		{
			if (currentFace != face)
			{
				glCullFace(static_cast<GLenum>(face));
				currentFace = face;
			}
		}
	private:
		Face currentFace = Face::Back;
	};

	class BlendCap : public Capability<GL_BLEND>
	{
	public:
		enum class Func
		{
			None, SrcAlpha_1MinuseSrcAlpha
		};
		void SetFunc(Func func)
		{
			if (currentFunc != func)
			{
				glBlendFunc(GetSrc(func), GetDst(func));
				currentFunc = func;
			}
		}
	private:
		static GLenum GetSrc(Func func)
		{
			using enum Func;
			switch (func)
			{
			case SrcAlpha_1MinuseSrcAlpha:
				return GL_SRC_ALPHA;
			default:
				return GL_NONE;
			}
		}
		static GLenum GetDst(Func func)
		{
			using enum Func;
			switch (func)
			{
			case SrcAlpha_1MinuseSrcAlpha:
				return GL_ONE_MINUS_SRC_ALPHA;
			default:
				return GL_NONE;
			}
		}
	private:
		Func currentFunc = Func::None;
	};

	class Capabilities
	{
	public:
		static DepthTestCap DepthTest();
		static CullFaceCap CullFace();
		static BlendCap Blend();
	private:
	};
}