export module Nork.Utils:Observed;

export namespace Nork {
	template<class T>
	struct Observed : T
	{
		Observed() = default;
		Observed(const T& value)
		{
			*(T*)this = value;
			copy = value;
		}
		Observed& operator=(const T& value)
		{
			*(T*)this = value;
			return *this;
		}
		Observed(const Observed& other) = delete;
		Observed(Observed&& other) = default;
		Observed& operator=(const Observed& other) = delete;
		Observed& operator=(Observed&& other) = delete;

		bool IsChanged(bool set = true)
		{
			bool changed;
			if constexpr (std::is_standard_layout<T>::value)
				changed = std::memcmp(this, &copy, sizeof(copy));
			else
				changed = (T&)*this != copy;
			if (set)
				copy = (T&)*this;
			return changed;
		}
	private:
		T copy; // must have operator== defined if not (is_standard_layout<T>::value == false)
	};
}