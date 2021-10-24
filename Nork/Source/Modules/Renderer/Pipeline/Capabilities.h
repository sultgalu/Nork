#pragma once

namespace Nork::Renderer::Capabilities
{
	enum class Capability
	{
		Depth = GL_DEPTH_TEST, Blend = GL_BLEND, FaceCulling = GL_CULL_FACE,
	};

	template<GLenum src, GLenum dst>
	struct BlendFunc
	{
		static void Enable()
		{
			glBlendFunc(src, dst);
		}
		static int Idx()
		{
			return 0;
		}
	};

	template<typename T>
	concept CapMode = requires(T t)
	{
		{ t.Enable() } -> std::same_as<void>;
		{ t.Idx() } -> std::same_as<int>;
	};

	class Data
	{
	private:
		struct Element
		{
			struct Cap
			{
				GLenum gl;
				uint8_t idx;
			};
			std::vector<Cap> caps;
			std::vector<uint8_t> changedModes;
		};
	public:
		static void Init()
		{
			for (size_t i = 0; i < modes.size(); i++)
			{
				modes[i].push([]() {});
			}
		}

		static void BeginPush()
		{
			static bool first = true;
			if (first)
			{
				first = false;
				Init();
			}

			stack.push({});
		}

		template<bool On, Capability Cap, Capability... Rest>
		static void Push()
		{
			if (caps.test(index<Cap>) != On)
			{
				if constexpr (On)
				{
					glEnable(asGL<Cap>);
				}
				else
				{
					glDisable(asGL<Cap>);
				}
				caps.flip(index<Cap>);
				stack.top().caps.push_back({asGL<Cap>, index<Cap>});
			}
			if constexpr (sizeof...(Rest) > 0)
				Push<On, Rest...>();
		}

		template<CapMode Mode, CapMode... Rest>
		static void Push()
		{
			if (modes[Mode::Idx()].top().target<void()>() != Mode::Enable)
			{
				stack.top().changedModes.push_back(Mode::Idx());
				modes[Mode::Idx()].push(Mode::Enable);
				Mode::Enable();
			}
			if constexpr (sizeof...(Rest) > 0)
				Push<Rest...>();
		}

		static void Pop()
		{
			for (size_t i = 0; i < stack.top().caps.size(); i++)
			{
				auto cap = stack.top().caps[i];
				if (caps.test(cap.idx))
				{
					glDisable(cap.gl);
				}
				else
				{
					glEnable(cap.gl);
				}
				caps.flip(cap.idx);
			}
			for (size_t i = 0; i < stack.top().changedModes.size(); i++)
			{
				auto& idx = stack.top().changedModes[i];
				modes[idx].pop();
				modes[idx].top()();
			}
			stack.pop();
		}

	private:
		inline static std::bitset<8> caps; // should know inital values
		inline static std::array<std::stack<std::function<void()>>, 5> modes; // should know inital values

		inline static std::stack<Element> stack;
	
		template<Capability Cap>
		inline static constexpr GLenum asGL = static_cast<GLenum>(Cap);

		template<Capability Cap>
		static constexpr uint8_t index;
		template<>
		static constexpr uint8_t index<Capability::Depth> = 0;
		template<>
		static constexpr uint8_t index<Capability::Blend> = 1;
		template<>
		static constexpr uint8_t index<Capability::FaceCulling> = 2;
	};

	template<Capability... _Caps>
	struct Enable
	{
		static void Push()
		{
			if constexpr (sizeof...(_Caps) > 0)
			{
				Data::Push<true, _Caps...>();
			}
		}
	};

	template<Capability... _Caps>
	struct Disable
	{
		static void Push()
		{
			if constexpr (sizeof...(_Caps) > 0)
			{
				Data::Push<false, _Caps...>();
			}
		}
	};

	template<CapMode... Modes>
	struct Set
	{
		static void Push()
		{
			if constexpr (sizeof...(Modes) > 0)
			{
				Data::Push<Modes...>();
			}
		}
	};

	template<typename T>
	concept CapContainer = requires(T t)
	{
		t.Push();
	};

	template<CapContainer T, CapContainer... Rest>
	static void PushAll()
	{
		T::Push();
		if constexpr (sizeof...(Rest) > 0)
			PushAll<Rest...>();
	}

	template<CapContainer... Caps>
	static void Push()
	{
		Data::BeginPush();
		if constexpr (sizeof...(Caps) > 0)
			PushAll<Caps...>();
	}

	static void Pop()
	{
		Data::Pop();
	}

	template<CapContainer... Caps, typename Func>
	static void With(Func f)
	{
		Push<Caps...>();
		f();
		Pop();
	}
}