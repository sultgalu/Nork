#pragma once

#include "BufferWrapper.h"

namespace Nork::Renderer {

	template<class T, BufferTarget _Target>
	class TypedBufferWrapper : BufferWrapper
	{
	public:
		TypedBufferWrapper(size_t initialCount = 0, std::source_location loc = std::source_location::current())
			: BufferWrapper(_Target, sizeof(T), initialCount)
		{
			Logger::Info("Created buffer wrapper from ", loc.file_name());
		}
		std::shared_ptr<size_t> Add(T* data, size_t count)
		{
			return BufferWrapper::Add(data, count);
		}
		std::shared_ptr<size_t> Add(const T& data)
		{
			return BufferWrapper::Add(&data, 1);
		}
		std::shared_ptr<size_t> Add(const std::vector<T>& data)
		{
			return BufferWrapper::Add(data.data(), data.size());
		}
		void Update(std::shared_ptr<size_t> idx, const T& data)
		{
			BufferWrapper::Update(idx, &data, 1);
		}
		using BufferWrapper::FreeSpace;
		using BufferWrapper::Prepare;
		using BufferWrapper::GetBuffer;
	};
}