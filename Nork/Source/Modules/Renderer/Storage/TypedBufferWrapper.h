#pragma once

#include "BufferWrapper.h"

namespace Nork::Renderer {

	template<class T, BufferTarget _Target>
	class TypedBufferWrapper : BufferWrapper
	{
	public:
		TypedBufferWrapper(size_t initialCount = 1000, std::source_location loc = std::source_location::current())
			: BufferWrapper(_Target, sizeof(T), initialCount)
		{
			Logger::Info("Created buffer wrapper from ", loc.file_name());
		}
		std::shared_ptr<T*> Add(T* data, size_t count)
		{
			return std::reinterpret_pointer_cast<T*>(BufferWrapper::Add(data, count));
		}
		std::shared_ptr<T*> Add(const std::vector<T>& data)
		{
			return std::reinterpret_pointer_cast<T*>(BufferWrapper::Add(data.data(), data.size()));
		}
		std::shared_ptr<T*> Add(const T& data)
		{
			return std::reinterpret_pointer_cast<T*>(BufferWrapper::Add(&data, 1));
		}
		size_t GetIdxFor(std::shared_ptr<T*> ptr)
		{
			return *ptr - (T*)BufferWrapper::GetPtr();
		}
		std::span<T> GetDirectData()
		{
			return std::span<T>((T*)BufferWrapper::GetPtr(), GetCount());
		}
		void MoveToBack(std::shared_ptr<T*> ptr)
		{
			return BufferWrapper::MoveToBack(GetIdxFor(ptr));
		}
		void Swap(std::shared_ptr<T*> ptr1, std::shared_ptr<T*> ptr2)
		{
			return BufferWrapper::Swap(GetIdxFor(ptr1), GetIdxFor(ptr2));
		}
		void Erase(std::shared_ptr<T*> ptr)
		{
			return BufferWrapper::Erase(GetIdxFor(ptr));
		}
		std::shared_ptr<T*> Back()
		{
			return std::reinterpret_pointer_cast<T*>(BufferWrapper::Back());
		}
		std::shared_ptr<T*> Front()
		{
			return std::reinterpret_pointer_cast<T*>(BufferWrapper::Front());
		}

		using BufferWrapper::Swap;
		using BufferWrapper::Erase;
		using BufferWrapper::FreeSpace;
		using BufferWrapper::GetBuffer;
		using BufferWrapper::GetSize;
		using BufferWrapper::GetCount;
	};

	template<class T>
	using UBO = TypedBufferWrapper<T, BufferTarget::UBO>;
	template<class T>
	using SSBO = TypedBufferWrapper<T, BufferTarget::SSBO>;
	template<class T>
	using VBO = TypedBufferWrapper<T, BufferTarget::Vertex>;
	using IBO = TypedBufferWrapper<uint32_t, BufferTarget::Index>;
}