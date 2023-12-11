#pragma once

namespace Nork
{
	template<class T>
	concept TriviallyCopyable = std::is_trivially_copyable<T>::value;

	namespace Template
	{
		namespace Types
		{
			struct Base {};

			struct NoConstruct : Base
			{
				NoConstruct() = delete;
			};
			struct NoCopyConstruct : Base
			{
				NoCopyConstruct() = default;
				NoCopyConstruct(const NoCopyConstruct&) = delete;
				NoCopyConstruct(NoCopyConstruct&) = delete;
			};
			struct NoCopyAssign : std::_Deleted_copy_assign<Base> {};
			struct NoMoveAssign : std::_Deleted_move_assign<Base> {};

			struct NoCopy : NoCopyAssign, NoCopyConstruct {};
			struct NoMove : NoMoveAssign {};

			struct OnlyConstruct : NoCopy, NoMove {};

			using Static = NoConstruct;

			struct VirtualType
			{
				virtual VirtualType* GetParent() const
				{
					return nullptr;
				}
				virtual size_t GetParentSize() const
				{
					return 0;	
				}
				size_t GetId() const // no need for virtual here
				{
					return typeid(*this).hash_code();
				}
				const char* GetName() const // no need for virtual here
				{
					return typeid(*this).name();
				}
				virtual std::vector<size_t> GetIds() const
				{
					return std::vector<size_t> { GetId() };
				}
				virtual std::vector<size_t> GetSizes() const
				{
					return std::vector<size_t> { GetParentSize() };
				}
				virtual std::vector<const char*> GetNames() const
				{
					return std::vector<const char*> { GetName() };
				}
			};

			template<std::derived_from<VirtualType> T>
			struct MetaInherited : T
			{
				using T::T; // using T's constructors

				virtual VirtualType* GetParent() const override
				{
					static T cached = static_cast<T>(*this);
					return &cached;
				}
				virtual size_t GetParentSize() const override
				{
					return sizeof(T);
				}
				virtual std::vector<size_t> GetIds() const override
				{
					/*auto getCached = [&]()
					{
						auto ids = this->GetParent()->GetIds();
						ids.push_back(this->GetId());
						return ids;
					};
					static auto cached = getCached();
					return cached;*/
					auto ids = this->GetParent()->GetIds();
					ids.push_back(this->GetId());
					return ids;
				}
				virtual std::vector<const char*> GetNames() const override
				{
					auto getCached = [&]()
					{
						auto names = this->GetParent()->GetNames();
						names.push_back(this->GetName());
						return names;
					};
					static auto cached = getCached();
					return cached;
				}
				virtual std::vector<size_t> GetSizes() const override
				{
					auto getCached = [&]()
					{
						auto sizes = this->GetParent()->GetSizes();
						sizes.push_back(this->GetParentSize());
						return sizes;
					};
					static auto cached = getCached();
					return cached;
				}
			};

		}

		namespace Utils
		{
			template<typename Element, size_t size, std::array<Element, size> arr>
			struct Loop
			{
				template<typename Func>
				Loop(Func func)
				{
					Recursive<0>(func);
				}

				Loop() = default;

				template<typename Func>
				consteval auto operator()(Func func)
				{
					return Recursive<0>(func);
				}

				template<size_t i, auto prev, typename Func>
				static consteval auto Recursive(Func func)
				{
					if constexpr (i < arr.size() - 1)
					{
						return Recursive<i + 1, func(i, prev)>(func);
					}
					return func(i, prev);
				}

				template<size_t i, typename Func>
				static consteval auto Recursive(Func func)
				{
					if constexpr (i < arr.size())
					{
						return Recursive<i + 1, func(i)>(func);
					}
				}
			};
		}
	}

	template<class T>
	concept EnumType = std::is_enum<T>::value;

	// For enum classes: Automatic implementation of bitwise operations
	/*template<EnumType T>
	constexpr T operator|(const T bit1, const T bit2)
	{
		return static_cast<T>(std::to_underlying(bit1) | std::to_underlying(bit2));
	}
	template<EnumType T>
	constexpr T operator&(const T bit1, const T bit2)
	{
		return static_cast<T>(std::to_underlying(bit1) & std::to_underlying(bit2));
	}*/

	/*template<EnumType T>
	constexpr T& operator|=(T& bit1, const T bit2)
	{
		return bit1 = bit1 | bit2;
	}
	template<EnumType T>
	constexpr T& operator&=(T& bit1, const T bit2)
	{
		return bit1 = bit1 & bit2;
	}

	template<EnumType T>
	constexpr T operator^(const T bit1, const T bit2)
	{
		return static_cast<T>(std::to_underlying(bit1) ^ std::to_underlying(bit2));
	}
	template<EnumType T>
	constexpr T& operator^=(T& bit1, const T bit2) noexcept
	{ 
		return bit1 = bit1 ^ bit2;
	}
	template<EnumType T>
	constexpr T operator~(const T bit1)
	{
		return static_cast<T>(~std::to_underlying(bit1));
	}*/
}