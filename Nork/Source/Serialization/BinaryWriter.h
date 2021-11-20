#pragma once
namespace Nork
{
	class BinaryWriter
	{
	public:
		template<TriviallyCopyable T>
		size_t operator<<(const T& d)
		{
			size_t size = sizeof(T);
			data.resize(data.size() + size);
			char* dest = data.data() + data.size() - size;

			*((T*)dest) = d;
			return size;
		}
		size_t operator<<(const std::string& str)
		{
			return Write(str.data(), str.size());
		}
		template<class T>
		size_t operator<<(const std::span<T> span)
		{
			return Write(span.data(), span.size_bytes());
		}
		template<class T>
		size_t operator<<(const std::vector<T>& vec)
		{
			return Write(vec.data(), vec.size() * sizeof(T));
		}
		template<TriviallyCopyable T>
		size_t Write(const T* d, size_t count)
		{
			size_t size = sizeof(T) * count;
			data.resize(data.size() + size);
			char* dest = data.data() + data.size() - size;

			std::memcpy(dest, &d, size);
			return size;
		}
		template<TriviallyCopyable T>
		T& WriteEmpty()
		{
			size_t size = sizeof(T);
			data.resize(data.size() + size);
			char* dest = data.data() + data.size() - size;
			return *((T*)dest);
		}
		template<TriviallyCopyable T>
		T& WriteEmpty(T val)
		{
			T& ref = WriteEmpty<T>();
			ref = val;
			return ref;
		}
		void Prepare(size_t size)
		{
			data.reserve(data.size() + size);
		}
		std::vector<char>& Data()
		{
			return data;
		}
	private:
		std::vector<char> data;
	};
}