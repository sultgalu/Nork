namespace Nork
{
	class BinaryReader
	{
	public:
		BinaryReader(std::span<char> buf) : buf(buf)
		{
			ptr = buf.data();
		}
		template<class T>
		BinaryReader(std::span<T> buf) : BinaryReader(std::span((char*)buf.data(), buf.size_bytes())) {}
		template<class T>
		BinaryReader(T* buf, size_t count) : BinaryReader(std::span(buf, count)) {}

		template<TriviallyCopyable T>
		BinaryReader& operator>>(T& val)
		{
			val = *((T*)ptr);
			ptr += sizeof(T);
			return *this;
		}
		template<TriviallyCopyable T>
		std::span<T> Read(size_t count)
		{
			auto res = std::span<T>((T*)ptr, count);
			ptr += res.size_bytes();
			return res;
		}
		std::string_view ReadStr(size_t size)
		{
			auto res = std::string_view(ptr, size);
			ptr += size;
			return res;
		}
		std::span<char> Data()
		{
			return buf;
		}
	private:
		char* ptr;
		std::span<char> buf;
	};
}
